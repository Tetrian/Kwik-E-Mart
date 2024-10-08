#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#include "./core/config.h"
#include "./core/checkout.h"
#include "./core/psql_api.h"
#include "./core/msg_handler.h"
#include "./helper/logging.h"
#include "./helper/ts_queue.h"
#include "./helper/signal_handler.h"
#include "netinet/in.h"
#include "libpq-fe.h"
#include "server.h"

/* Control switch for gracefully shutdown (NB: This variable has to be
 * volative!) */
volatile sig_atomic_t _keep_alive = 1;

static void handle_interrupt(int signal);

server_t *init_server(unsigned int port, const char *db_conn_info,
                      const size_t max_clients, const size_t max_workers,
                      const size_t max_jobs, void *(*routine)(void *)) {
  server_t *s = malloc(sizeof(struct server_t));
  if (s == NULL) {
    log_error("[%s] (%s) Failed to allocate space for the server\n",
              __FILE_NAME__, __func__);
    return NULL;
  }

  s->socket = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
  if (s->socket == -1) {
    log_error("[%s] (%s) Failed to connect socket! Cause: %s\n", __FILE_NAME__,
              __func__, strerror(errno));
    perror("socket: ");
    free(s);
    return NULL;
  }

  log_info("[%s] (%s) Socket <%ld> created!\n", __FILE_NAME__, __func__,
           s->socket);
  /* Socket options (Set socket as reusable) */
  if (setsockopt(s->socket, SOL_SOCKET, SO_REUSEADDR, (int[]){1},
                 sizeof(int[1]))) {
    log_error("Could not change socket settings! Cause: %s\n", strerror(errno));
    perror("setsockopt: ");
    free(s);
    return NULL;
  }

  log_info("[%s] (%s) Socket <%ld> set as reusable!\n", __FILE_NAME__, __func__,
           s->socket);

  memset(&s->transport, 0, sizeof(s->transport));

  s->transport.sin_family = AF_INET;
  s->transport.sin_addr.s_addr = htonl(INADDR_ANY);
  s->transport.sin_port = htons(port);

  // Initialization of the variables for contingency management
  if (pthread_mutex_init(&(s->mx), NULL) != 0) {
    log_error("[%s] (%s) Could not initializate the mutex! Cause: %s\n", 
              __FILE_NAME__, __func__, strerror(errno));
    perror("pthread_mutex_init: ");
		free(s);
    return NULL;
  }
  s->max_clients = max_clients;
  s->clients_on = 0;

  /** Socket Binding
  The INADDR_ANY binding does not generate a random IP address
  It maps the socket to all available interfaces (which on a server for obvious
          reasons is something strongly desired), and not just localhost.
  */
  if (bind(s->socket, (struct sockaddr *)&s->transport, sizeof(s->transport)) !=
      0) {
    log_error("[%s] (%s) Could not bind the server! Cause: %s\n", __FILE_NAME__,
              __func__, strerror(errno));
    perror("bind: ");
    destroy_server(s);
    return NULL;
  }
  log_info("[%s] (%s) Server binded to socket <%d>!\n", __FILE_NAME__, __func__,
           s->socket);
  
  // Init  value for conditional jump in destroy servers
  s->queue = NULL;
  s->workers = NULL;
  s->checkouts = NULL;

  // Initialization of the database
  if ((s->db = init_db(db_conn_info)) == NULL) {
    log_error("[%s] (%s) Failed to initializate the db!\n",
              __FILE_NAME__, __func__);
    destroy_server(s);
    return NULL;
  }
  log_info("[%s] (%s) Database initializated and server connected to it.\n",
            __FILE_NAME__, __func__);

  // get the products list
  s->products[0] = '\0';
  if (get_all_products(s->db, s->products) == 0) {
    log_error("[%s] (%s) Failed to get product list.\n",
              __FILE_NAME__, __func__);
    destroy_server(s);
    return NULL;
  }

  // Initialization of the variables for the clients connection
  if ((s->queue = init_queue()) == NULL) {
    log_error("[%s] (%s) Failed to allocate enough space for the "
              "server->clients! Cause: %s\n",
              __FILE_NAME__, __func__, strerror(errno));
    destroy_server(s);
    return NULL;
  }

  // Socket listening
  if (listen(s->socket, max_clients) != 0) {
    log_error("[%s] (%s) Could not start listening! Cause: %s\n", __FILE_NAME__,
              __func__, strerror(errno));
    perror("listen: ");
    destroy_server(s);
    return NULL;
  }
  log_info("[%s] (%s) Server ready and it is listening for new clients on "
           "port: %d!\n",
           __FILE_NAME__, __func__, port);
  if ((s->workers = malloc(sizeof(pthread_t[max_workers]))) == NULL) {
    log_error("[%s] (%s) Failed to allocate enough space for the "
              "server->pool! Cause: %s\n",
              __FILE_NAME__, __func__, strerror(errno));
    destroy_server(s);
    return NULL;
  }

  s->max_workers = max_workers;
  for (size_t i = 0; i < max_workers; i++) {
    if (pthread_create(&(s->workers[i]), NULL, routine, (void *)s) != 0) {
      log_error("[%s] (%s) Failed to spawn thread n° %zu! Cause: %s\n",
                __FILE_NAME__, __func__, i + 1, strerror(errno));
      perror("pthread_create: ");
      destroy_server(s);
      return NULL;
    }
    log_info("[%s] (%s) Spawned thread n° %zu\n", __FILE_NAME__, __func__,
             i + 1);
  }

  //Initialization of the supermarket checkouts
  if ((s->checkouts = malloc(sizeof(checkout_t[max_jobs]))) == NULL) {
    log_error("[%s] (%s) Failed to allocate enough space for the "
              "server->checkouts! Cause: %s\n",
              __FILE_NAME__, __func__, strerror(errno));
    destroy_server(s);
    return NULL;
  }
  
  s->max_checkouts = max_jobs;
  for (size_t i = 0; i < max_jobs; i++) {
    s->checkouts[i] = init_checkout();
    if (s->checkouts[i] == NULL) {
      log_error("[%s] (%s) Failed to init checkout n° %zu!\n",
                __FILE_NAME__, __func__, i + 1);
      destroy_server(s);
      return NULL;
    }
    log_info("[%s] (%s) Added checkout n° %zu\n", __FILE_NAME__, __func__,
             i + 1);
  }

  log_info("[%s] (%s) Server ready, please use server_loop!\n", __FILE_NAME__,
           __func__);

  return s;
}

void destroy_server(server_t *s) {
  if (s == NULL) return;

  log_info("[%s] (%s) Shutting down the server <%ld> as requested\n",
          __FILE_NAME__, __func__, s->socket);
  close(s->socket);

  log_info("[%s] (%s) Destroying server->checkouts\n", __FILE_NAME__, 
            __func__);
  for (size_t i = 0; i < s->max_checkouts; i++)
    destroy_checkout(s->checkouts[i]);
  free(s->checkouts);

  if (s->workers != NULL) {
    log_info("[%s] (%s) Destroying server->pool\n", __FILE_NAME__, __func__);

    // free any blocked threads
    wake_all(s->queue);

    log_info("[%s] (%s) Waiting the threads\n", __FILE_NAME__, __func__);
    for (size_t i = 0; i < s->max_workers; i++) {
      if (pthread_join(s->workers[i], NULL) != 0)
        log_error("[%s] (%s) Failed to join thread n° %zu! Cause: %s\n",
                  __FILE_NAME__, __func__, i + 1, strerror(errno));
    }
    free(s->workers);
  }

  log_info("[%s] (%s) Destroying server->queue\n", __FILE_NAME__, __func__);
  destroy_queue(s->queue);

  log_info("[%s] (%s) Destroying server->db\n", __FILE_NAME__, __func__);
  destroy_db(s->db);
  
  pthread_mutex_destroy(&(s->mx));
  free(s);

  log_info("[%s] (%s) Goodbye!\n", __FILE_NAME__, __func__);
}

void server_loop(server_t *s) {
  if (s==NULL) {
    log_error("[%s] (%s) Failed to initialize the Server! Cause: %s\n",
              __FILE_NAME__, __func__, strerror(errno));
    return;
  }
  log_info("[%s] (%s) Server is now setupping signals in order to get "
           "gracefully shutdown! (To make a hard shutdown kill pid n° %d)\n",
           __FILE_NAME__, __func__, getpid());
  setup_signals(SIGINT, handle_interrupt);

  socklen_t len = sizeof(s->transport);
  while (_keep_alive) {
    ssize_t sd = accept(
               s->socket, (struct sockaddr *)&s->transport, &len);
    if (sd != -1) {
      pthread_mutex_lock(&(s->mx));
      if (s->clients_on < s->max_clients) {
        s->clients_on++;
        pthread_mutex_unlock(&(s->mx));
        log_info("[%s] (%s) Client connected on socket n°%d\n", 
                 __FILE_NAME__, __func__, sd);
        if (write_msg(sd, ACK, NULL) == -1) {
          log_error("[%s] (%s) Socket n°%d is broken. End communication.\n",
                      __FILE_NAME__, __func__, sd);
        }
        enqueue(s->queue, (void *)sd);
      }
      else {
        pthread_mutex_unlock(&(s->mx));
        if (write_msg(sd, NAK, NULL) == -1) {
          log_error("[%s] (%s) Socket n°%d is broken. End communication.\n",
                      __FILE_NAME__, __func__, sd);
        }
        log_info("[%s] (%s) Connection on socket n°%d refused.\n", 
                 __FILE_NAME__, __func__, sd);
      }
    }
  }
}

/** Handle server to client communication */
void *connection_handler(void *sd) {
  while (_keep_alive) {
    // Parsing of the socket descriptor
    ssize_t socket = (ssize_t)dequeue(((server_t *)sd)->queue);
    if (!is_keep(((server_t *)sd)->queue)) return NULL;

    log_info(
        "[%s] (%s) Start communication with the client on socket n°%d.\n",
        __FILE_NAME__, __func__, socket);
    server_t *s = ((server_t *)sd);

    // Start loop for message management
    bool flag = true;
    while (_keep_alive && flag) {
      // read the request
      uint8_t request[BUFFSIZE] = {0};
      ssize_t readed_bytes = read(socket, request, BUFFSIZE);
      if (readed_bytes == -1) {
        log_error("[%s] (%s) Socket n°%d is broken. End communication.\n",
                  __FILE_NAME__, __func__, socket);
        pthread_mutex_lock(&(((server_t *)sd)->mx));
        ((server_t *)sd)->clients_on--;
        pthread_mutex_unlock(&(((server_t *)sd)->mx));
        return NULL;
      }

      // handle the request
      if (is_valid(request, readed_bytes)) {
        switch (request[CMD_POS]) {
          case BEL: // allow permission for enter in the shop
            if (write_msg(socket, BEL, s->products) == -1) {
              log_error("[%s] (%s) Socket n°%d is broken. End communication.\n",
                        __FILE_NAME__, __func__, socket);
              pthread_mutex_lock(&(((server_t *)sd)->mx));
              ((server_t *)sd)->clients_on--;
              pthread_mutex_unlock(&(((server_t *)sd)->mx));
              return NULL;
            }
            log_info("[%s] (%s) Socket n°%d is inside the market\n",
                    __FILE_NAME__, __func__, socket);
            break;
          
          case SI: // received request for checkouts
            char receipt[readed_bytes - WRAPSIZE + 1];
            receipt[readed_bytes - WRAPSIZE] = '\0';
            parse_payload(request, receipt, readed_bytes);
            
            char *price;
            size_t n_products = (size_t)strtol(receipt, &price, 10);

            // If there is a purchased, handle the checkout
            if (n_products > 0) {
              log_info("[%s] (%s) Socket n°%d is in queue for checkout\n",
                    __FILE_NAME__, __func__, socket);
              int delay = enter_checkout(s->checkouts[socket%s->max_checkouts]);
              
              sleep(delay + (int)(n_products/2));
              leave_checkout(s->checkouts[socket%s->max_checkouts]);
              
              pthread_mutex_lock(&(((server_t *)sd)->mx));
              insert_receipt(((server_t *)sd)->db, price + 1);
              pthread_mutex_unlock(&(((server_t *)sd)->mx));
            }
            if (write_msg(socket, SO, NULL) == -1) {
              log_error("[%s] (%s) Socket n°%d is broken. End communication.\n",
                        __FILE_NAME__, __func__, socket);
              pthread_mutex_lock(&(((server_t *)sd)->mx));
              ((server_t *)sd)->clients_on--;
              pthread_mutex_unlock(&(((server_t *)sd)->mx));
              return NULL;
            }

            flag = false; // exit from the loop
            break;
        }
      } 
    }

    pthread_mutex_lock(&(((server_t *)sd)->mx));
    ((server_t *)sd)->clients_on--;
    pthread_mutex_unlock(&(((server_t *)sd)->mx));
    log_info("[%s] (%s) Socket n°%d exit from the shop\n",
                    __FILE_NAME__, __func__, socket);
  }
  return NULL;
}

static void handle_interrupt(int signal) {
  // Ignore the value of the signal -> We know that if the signal is caught the
  // server is asked to shutdown
  (void)signal;
  log_info("[%s] (%s) The server is shutting down... Goodbye!\n", __FILE_NAME__,
           __func__, signal);
  _keep_alive = 0;
}

/* ************************************************************************** */

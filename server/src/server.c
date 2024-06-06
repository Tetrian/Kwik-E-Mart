#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#include "./core/config.h"
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
                      const size_t max_connections,void *(*routine)(void *)) {
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
  if (get_all_products(s->db, s->products) == 0) {
    log_error("[%s] (%s) Failed to get product list.\n",
              __FILE_NAME__, __func__);
    destroy_server(s);
    return NULL;
  }
  
  //TODO: Initialization of the supermarket checkouts
  //NOTE: U can use max_connection for the db_pool too
  (void)max_connections;

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

  log_info("[%s] (%s) Server ready, please use server_loop!\n", __FILE_NAME__,
           __func__);

  return s;
}

void destroy_server(server_t *s) {
  if (s == NULL) return;

  log_info("[%s] (%s) Shutting down the server <%ld> as requested\n",
          __FILE_NAME__, __func__, s->socket);
  close(s->socket);

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
      log_info("[%s] (%s) Client connected on socket n°%d\n", __FILE_NAME__,
                 __func__, sd);
      enqueue(s->queue, (void *)sd);
    }
  }
}

/*
 * Gestisce la comunicazione tra server e client
 */
void *connection_handler(void *sd) {
  // Parsing del socket descriptor
  ssize_t socket = (ssize_t)dequeue(((server_t *)sd)->queue);
  if (!is_keep(((server_t *)sd)->queue)) return NULL;

  log_info(
      "[%s] (%s) Start communication with the client on socket n°%d.\n",
      __FILE_NAME__, __func__, socket);
  server_t *s = ((server_t *)sd);
  
  log_info("[%s] (%s) Sending products list to socket n°%d.\n",
           __FILE_NAME__, __func__, socket);
  
  char ack;
  do {
    write_msg(socket, BEL, s->products);
    read(socket, &ack, 1);
  } while (ack != ACK);
  
  log_info("[%s] (%s) Socket n°%d is inside the market\n",
           __FILE_NAME__, __func__, socket);

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

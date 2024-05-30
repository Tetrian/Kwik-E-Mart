
#include <arpa/inet.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "./core/config.h"
#include "./helper/logging.h"
#include "server.h"

// number of customers that can making purchases
#define MAX_CUSTOMERS 10

// number of cashiers TODO: da gestire
#define MAX_CASHIER 3

#define DB_CONN_INFO                                                           \
  "host=localhost port=5432 dbname=postgres user=root password=password "      \
  "connect_timeout=10"

int main() {
  server_t *s = init_server(PORT, DB_CONN_INFO, MAX_CUSTOMERS, MAX_CUSTOMERS,
                            connection_handler);
  server_loop(s);
  destroy_server(s);
}

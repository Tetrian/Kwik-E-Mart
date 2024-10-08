
#include "./core/config.h"
#include "server.h"

// number of customers that can making purchases
#define MAX_CUSTOMERS 10

// number of cashiers
#define MAX_CASHIER 3

#define DB_CONN_INFO                                                           \
  "host=localhost port=5454 dbname=postgres user=root password=password "      \
  "connect_timeout=10"

int main() {
  server_t *s = init_server(PORT, DB_CONN_INFO, MAX_CUSTOMERS, MAX_CUSTOMERS,
                            MAX_CASHIER, connection_handler);
  server_loop(s);
  destroy_server(s);
}

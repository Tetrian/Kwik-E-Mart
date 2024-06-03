#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>

#include "psql_api.h"
#include "libpq-fe.h"
#include "config.h"
#include "../helper/logging.h"

/* 
 * Make a connection to the database server and create tables
https://www.postgresql.org/docs/8.1/libpq.html#LIBPQ-CONNECT
*/
// TODOS: restituire la struct
PGconn *init_db(const char *conninfo) {
  // Create the connection
  PGconn *conn = PQconnectdb(conninfo);
  if (PQstatus(conn) != CONNECTION_OK) {
    log_error("[%s] (%s) Connection to database failed: %s", __FILE_NAME__,
              __func__, PQerrorMessage(conn));
    PQfinish(conn);
    return NULL;
  }
  
  //TODO:inizializza la struttura

  // Create tables
  if (!create_table(conn, CRT_PRODUCT_TBL))
    return NULL;

  // popola la tabella prodotti se non lo è già
  // N.B.: usare e gestire un bool che te lo dica

  if (!create_table(conn, CRT_RECEIPT_TBL))
    return NULL;

  return conn;
}

/** exit nicely */
void cleanup_db(PGconn *conn, PGresult *res) {
  PQclear(res);
  PQfinish(conn);
}

/** create a table for the database */
bool create_table(PGconn *conn, const char *create_cmd) {
  PGresult *res = PQexec(conn, create_cmd);
  if (PQresultStatus(res) != PGRES_COMMAND_OK) {
    log_error("[%s] (%s) CREATE failed: %s",__FILE_NAME__,
              __func__, PQerrorMessage(conn));
    cleanup_db(conn, res);
    return false;
  }
  PQclear(res);
  return true;
}
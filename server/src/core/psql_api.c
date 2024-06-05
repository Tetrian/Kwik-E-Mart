#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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
db_t *init_db(const char *conninfo) {
  db_t *db = malloc(sizeof(db_t));
  if (db == NULL) {
    log_error("[%s] (%s) Failed to allocate space for the db.\n",
              __FILE_NAME__, __func__);
    return NULL;
  }

  // Create the connection
  db->conn = PQconnectdb(conninfo);
  if (PQstatus(db->conn) != CONNECTION_OK) {
    log_error("[%s] (%s) Connection to database failed: %s", __FILE_NAME__,
              __func__, PQerrorMessage(db->conn));
    PQfinish(db->conn);
    free(db);
    return NULL;
  }

  // init mutex
  if (pthread_mutex_init(&(db->mutex), NULL) != 0) {
    log_error("[%s] (%s) Could not initializate the mutex! Cause: %s\n", 
              __FILE_NAME__, __func__, strerror(errno));
    perror("pthread_mutex_init: ");
		PQfinish(db->conn);
    free(db);
    return NULL;
  }

  // Create tables
  if (!create_table(db->conn, CRT_PRODUCT_TBL)) {
    destroy_db(db);
    return NULL;
  }

  if (!create_table(db->conn, CRT_RECEIPT_TBL)) {
    destroy_db(db);
    return NULL;
  }

  // population of table product
  log_info("[%s] (%s) Starting populate the products table...\n",
           __FILE_NAME__, __func__);
  populate_product_tbl(db);

  return db;
}

void destroy_db(db_t *db) {
  if (db != NULL) {
    PQfinish(db->conn);
    pthread_mutex_destroy(&(db->mutex));
    free(db);
  }
}

/** create a table for the database */
bool create_table(PGconn *conn, const char *create_cmd) {
  PGresult *res = PQexec(conn, create_cmd);
  if (PQresultStatus(res) != PGRES_COMMAND_OK) {
    log_error("[%s] (%s) CREATE failed: %s",__FILE_NAME__,
              __func__, PQerrorMessage(conn));
    PQclear(res);
    return false;
  }
  PQclear(res);
  return true;
}

/** populate the product table */
void populate_product_tbl(db_t * db) {
  // check if product is already populate
  PGresult *res = PQexec(db->conn, "SELECT COUNT(*) FROM product");
  if (strtol(PQgetvalue(res, 0, 0), NULL, 10) > 0) {
    PQclear(res);
    log_info("[%s] (%s) Table 'product' isn't empty, skipping\n",
             __FILE_NAME__, __func__);
    return;
  }

  // declare products name
  const char *const names[] = {"Duff Beer", "KrustyO's", "Pizza", 
                  "Donuts", "Buzz Cola", "Slurpee", "Pollo"};
  size_t size = 7;

  for (size_t i = 0; i < size; ++i) {
    // generate random price
    float price = (float)rand()/((float)RAND_MAX/PRICE_MAX);

    insert(db, INSERT_PRODUCT, i+1, names[i], price);
  }
}


/*
 * insert a element into a database
 * @param db database struct
 * @param cmd insert command to be executed
 * @param id object unic id
 * @param str name of the product or date of the receipt
 * @param price the price of product or total price of receipt
 */
void insert(db_t *db, const char *cmd, const int id,
            const char *str, const float price) 
{ 
  // Parsing the id and the price
  char cid[ENOUGHT];
  sprintf(cid, "%d", id);
  char cprice[ENOUGHT];
  sprintf(cprice, "%.2f", (double)price);

  // Specific the values and lenghts of parameters
  const char *const param_values[] = {cid, str, cprice};
  const int param_lengths[] = {sizeof(cid), sizeof(str), sizeof(cprice)};

  pthread_mutex_lock(&(db->mutex));

  // https://www.postgresql.org/docs/current/libpq-exec.html#LIBPQ-PQEXECPARAMS
  PGresult *res = PQexecParams(db->conn, cmd, N_PARAMS,
                      NULL, param_values, param_lengths, 
                      NULL, TEXT_FORMAT
  );

	if (PQresultStatus(res) != PGRES_COMMAND_OK)
		log_error("[%s] (%s) INSERT failed: %s",__FILE_NAME__,
              __func__, PQerrorMessage(db->conn));
	PQclear(res);

  pthread_mutex_unlock(&(db->mutex));
}


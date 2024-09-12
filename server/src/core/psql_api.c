#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <pthread.h>
#include <time.h>

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
  if (get_last_id(db, "product") > 0) {
    log_info("[%s] (%s) Table 'product' isn't empty, skipping\n",
             __FILE_NAME__, __func__);
    return;
  } 

  // declare products name
  const char *const names[] = {"Duff Beer", "KrustyO's", "Pizza", 
                  "Donut", "Buzz Cola", "Slurpee", "Chicken"};
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
		log_error("[%s] (%s) INSERT failed: %s\n",__FILE_NAME__,
              __func__, PQerrorMessage(db->conn));
	PQclear(res);

  pthread_mutex_unlock(&(db->mutex));
}

/*
 * parse the receipt and insert it into database
 * @param db database struct
 * @param total string that contain the price
 */
void insert_receipt(db_t *db, const char *total) {  
  // set the new id
  int id = get_last_id(db, "receipt") + 1;
  char cid[ENOUGHT];
  sprintf(cid, "%d", id);
  
  // get time
  char date[ENOUGHT*ENOUGHT];
  time_t now = time(NULL);
  struct tm *t = localtime(&now);
  strftime(date, sizeof(date)-1, "%d/%m/%Y %H:%M", t);

  // Specific the values and lenghts of parameters
  const char *const param_values[] = {cid, date, total};
  const int param_lengths[] = {sizeof(cid), sizeof(date), sizeof(total)};

  pthread_mutex_lock(&(db->mutex));
  PGresult *res = PQexecParams(db->conn, INSERT_RECEIPT, N_PARAMS,
                      NULL, param_values, param_lengths, 
                      NULL, TEXT_FORMAT
  );
	if (PQresultStatus(res) != PGRES_COMMAND_OK)
		log_error("[%s] (%s) INSERT failed: %s\n",__FILE_NAME__,
              __func__, PQerrorMessage(db->conn));
	PQclear(res);
  pthread_mutex_unlock(&(db->mutex));
}

/*
 * get name and price of all products in the format
 * name€price$
 * @param db database struct
 * @param str string to save the products, must be ""
 * @return the number of product getted
 */
int get_all_products(db_t *db, char *str) {
  pthread_mutex_lock(&(db->mutex));
  PGresult *res = PQexec(db->conn, "SELECT name, price FROM product");
  pthread_mutex_unlock(&(db->mutex));

  int rows = PQntuples(res);
  for (int i = 0; i < rows; ++i) {
    strcat(str, PQgetvalue(res, i, 0));
    strcat(str, "€");
    strcat(str, PQgetvalue(res, i, 1));
    strcat(str, "$");
  }

  PQclear(res);
  return rows;
}

/*
 * get the number of last id
 * @param db database struct
 * @param table_name name of the table
 * @return the number of last id
 */
int get_last_id(db_t* db, const char *table_name) {
  char cmd[CMDSIZE] = "SELECT COUNT(*) FROM ";
  strcat(cmd, table_name);

  pthread_mutex_lock(&(db->mutex));
  PGresult *res = PQexec(db->conn, cmd);
  pthread_mutex_unlock(&(db->mutex));

  int nids = strtol(PQgetvalue(res, 0, 0), NULL, 10);
  PQclear(res);
  
  return nids;
}
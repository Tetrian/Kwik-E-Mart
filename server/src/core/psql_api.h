#ifndef PSQL_API_H
#define PSQL_API_H

#include <stdbool.h>
#include <pthread.h>

#include "libpq-fe.h"

/* ************************************************************************** */

// Create table commands
#define CRT_PRODUCT_TBL \
    "CREATE TABLE IF NOT EXISTS product(id INTEGER PRIMARY KEY," \
    "name VARCHAR(20), price FLOAT)"

#define CRT_RECEIPT_TBL \
    "CREATE TABLE IF NOT EXISTS receipt(id INTEGER PRIMARY KEY," \
    "date VARCHAR(20), total_price FLOAT)"

// Insert table commands
#define INSERT_PRODUCT "INSERT INTO product VALUES($1, $2, $3)"

#define INSERT_RECEIPT "INSERT INTO receipt VALUES($1, $2, $3)"

// Result format for PQexecParams
#define TEXT_FORMAT 0
#define BINARY_FORMAT 1

// number of parameters for both tables
#define N_PARAMS 3
#define ENOUGHT 10 // plenty size for id and price string
#define PRICE_MAX 100 // max price of a single product

/* ************************************************************************** */

typedef struct db_t {
    pthread_mutex_t mutex;
    PGconn *conn;
} db_t;

/* ************************************************************************** */

// Initialization functions

db_t *init_db(const char*);

void destroy_db(db_t *);

void cleanup_db(PGconn*, PGresult*);

bool create_table(PGconn*, const char*);

void populate_product_tbl(db_t *);

/* ************************************************************************** */

// data operations

void insert(db_t *, const char *, const int, const char *, const float);
//      insert_receipt (va sincronizzata)
//      get_all_products (va sincronizzata se la uso nei thread altrimenti)
//                       (posso chiamarla prima e salvarla nel server)

/* ************************************************************************** */

#endif
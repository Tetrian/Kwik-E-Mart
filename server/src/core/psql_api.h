#ifndef PSQL_API_H
#define PSQL_API_H

#include <stdbool.h>
#include <pthread.h>

#include "libpq-fe.h"

/* ************************************************************************** */

#define CRT_PRODUCT_TBL \
    "CREATE TABLE IF NOT EXISTS product(id INTEGER PRIMARY KEY," \
    "name VARCHAR(20), price FLOAT)"

#define CRT_RECEIPT_TBL \
    "CREATE TABLE IF NOT EXISTS product(id INTEGER PRIMARY KEY," \
    "date VARCHAR(20), total_price FLOAT)"

/* ************************************************************************** */

//TODO: Struct per la gestione delle operazioni sul db
//          PGconn
//          mutex
//          condition variable
//          bool per product già popolata

/* ************************************************************************** */

// Initialization functions

PGconn *init_db(const char*);

void cleanup_db(PGconn*, PGresult*);

bool create_table(PGconn*, const char*);

//TODO: funzione per popolare la tabella PRODUCT

//TODO: funzione per distruggere la struttura

/* ************************************************************************** */

// data operations

//TODO: insert_product (non ha bisogno di sincronizzazione)
//      insert_receipt (va sincronizzata)
//      get_all_products (va sincronizzata se la uso nei thread altrimenti)
//                       (posso chiamarla prima e salvarla nel server)

/* ************************************************************************** */

#endif
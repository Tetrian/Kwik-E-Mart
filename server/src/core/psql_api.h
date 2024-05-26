#ifndef PSQL_API_H
#define PSQL_API_H

#include <stdbool.h>

#include "libpq-fe.h"

/* ************************************************************************** */

// MACRO HERE

/* ************************************************************************** */

// Initialization functions

PGconn *init_db(const char*);

void cleanup_db(PGconn*, PGresult*);

bool create_table(PGconn*, const char*);

/* ************************************************************************** */

// data operations

/* ************************************************************************** */

#endif
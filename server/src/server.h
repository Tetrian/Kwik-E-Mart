#ifndef SERVER_H
#define SERVER_H

#include <arpa/inet.h>
#include <pthread.h>

#include "./helper/ts_queue.h"
#include "./core/psql_api.h"
#include "libpq-fe.h"

/* ************************************************************************** */

// Structure to manage the connection with the client
typedef struct server_t {
	ssize_t socket;
	struct sockaddr_in transport;
	db_t *db;
	//TODO: add checkouts pool
	pthread_t *workers;
	ts_queue_t *queue;
	size_t max_workers;
} server_t;

// Basic structures for server management
server_t* init_server(unsigned int, const char*, const size_t, const size_t, const size_t, void *(*) (void *));
void destroy_server(server_t*);
void server_loop(server_t*);

// Manages the connection for each client
void *connection_handler(void*);

/* ************************************************************************** */

#endif

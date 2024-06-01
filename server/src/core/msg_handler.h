#ifndef MSG_HANDLER_H
#define MSG_HANDLER_H

#include <stdbool.h>
#include <stdint.h>
#include <sys/socket.h>

/* ************************************************************************** */

// ASCII control characters
#define SOH 1
#define ACK 6
#define BEL 7
#define CR  13
#define SO  14
#define SI  15
#define NAK 21

// Buffer max size
#define BUFFSIZE 250

// Num of byte in the payload without args
#define WRAPSIZE 4 // SOH + CODE + CR + CS

// pkg = 2byte + MSG + 2byte
#define MSGBOUND 2

/* ************************************************************************** */

ssize_t write_msg(int, const u_int8_t, const char *);

ssize_t read_msg(int, const u_int8_t, char *);

/* ************************************************************************** */

void create_payload(u_int8_t *, u_int8_t, const char *);

void parse_payload(u_int8_t *, char *, const size_t);

bool is_valid(u_int8_t *, const u_int8_t, const size_t);

/* ************************************************************************** */

#endif
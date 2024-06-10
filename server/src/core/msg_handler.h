#ifndef MSG_HANDLER_H
#define MSG_HANDLER_H

#include <stdbool.h>
#include <stdint.h>
#include <sys/socket.h>

/* ************************************************************************** */

// ASCII control characters
#define SOH 1
#define BEL 7
#define CR  13
#define SO  14
#define SI  15

// Buffer max size
#define BUFFSIZE 250

// Num of byte in the payload without args
#define WRAPSIZE 4 // SOH + CODE + CR + CS

// pkg = 2byte + MSG + 2byte
#define MSGBOUND 2

// position of the command code
#define CMD_POS 1

/* ************************************************************************** */

ssize_t write_msg(int, const uint8_t, const char *);

/* ************************************************************************** */

size_t create_payload(uint8_t *, uint8_t, const char *);

void parse_payload(const uint8_t *, char *, const size_t);

bool is_valid(const uint8_t *, const size_t);

/* ************************************************************************** */

#endif
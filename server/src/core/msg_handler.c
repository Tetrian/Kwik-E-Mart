
#include <stdbool.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/socket.h>

#include "msg_handler.h"

ssize_t write_msg(int sock, const u_int8_t code, const char *msg) {
    size_t size = (msg == NULL)? WRAPSIZE : WRAPSIZE + strlen(msg);
    u_int8_t payload[size];
    create_payload(payload, code, msg);

    ssize_t written_bytes = write(sock, payload, size);
    return written_bytes;
}

ssize_t read_msg(int sock, const u_int8_t expected_code, char *msg) {
    uint8_t payload[BUFFSIZE];
    ssize_t readed_bytes = read(sock, payload, BUFFSIZE);

    if (readed_bytes != -1) {
        // write ack if payload is valid, nak otherwise
        if (!is_valid(payload, expected_code, readed_bytes)) {
            int nak = NAK;
            write(sock, &nak, 1);
            return -1;
        }
        int ack = ACK;
        write(sock, &ack, 1);

        parse_payload(payload, msg, readed_bytes);
    }
    
    return readed_bytes;
}

void create_payload(u_int8_t *payload, const u_int8_t code, const char *msg) {
    size_t i = 0;
    payload[i++] = (u_int8_t)SOH;
    payload[i++] = code;
    u_int8_t checksum = payload[0] + payload[1];
    if (msg != NULL) {
        for (size_t j; j < strlen(msg); ++j) {
            uint8_t byte = (uint8_t)msg[j];
            checksum += byte;
            payload[i++] = byte;
        }
    }
    payload[i++] = checksum;
    payload[i] = (uint8_t)CR;
}

void parse_payload(u_int8_t *payload, char *msg, const size_t size) {
    if (msg != NULL) {
        for(size_t i = MSGBOUND; i < size - MSGBOUND; ++i)
            msg[i - MSGBOUND] = (char)payload[i];
    }
}

bool is_valid(u_int8_t *payload, const u_int8_t expected_code, const size_t size) {
    size_t i = 0;
    if (payload[i++] != (u_int8_t)SOH || 
        payload[i++] != expected_code ||
        payload[size - 1] != (u_int8_t)CR
    )
        return false;
    
    u_int8_t checksum;
    while (payload[i+1] == (u_int8_t)CR) {
        checksum += payload[i++];
    }

    return checksum == payload[i];
}

#include <stdbool.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/socket.h>

#include "msg_handler.h"

/*
 * write a message to a socket
 * @param sock socket descriptor
 * @param code control character
 * @param msg the message to send
 * @return On success, the number of bytes written is returned.
 *         On error, -1 is returned
 */
ssize_t write_msg(int sock, const uint8_t code, const char *msg) {
    size_t size = (msg == NULL)? WRAPSIZE : WRAPSIZE + strlen(msg);
    uint8_t payload[size];
    create_payload(payload, code, msg);

    ssize_t written_bytes = write(sock, payload, size);
    return written_bytes;
}

/*
 * read a message from a socket
 * @param sock socket descriptor
 * @param expected_code control character expected
 * @param buffer of char where to save the message
 * @return On success, the number of bytes readed is returned.
 *         On error, -1 is returned
 */
ssize_t read_msg(int sock, const uint8_t expected_code, char *msg) {
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

/*
 * generate the payload from msg
 * @param payload buffer of bytes where to save the payload
 * @param code control character
 * @param msg the real message of the payload
 * @return the size of the payload
 */
size_t create_payload(uint8_t *payload, const uint8_t code, const char *msg) {
    size_t i = 0;
    payload[i++] = (uint8_t)SOH;
    payload[i++] = code;
    uint8_t checksum = payload[0] + payload[1];
    if (msg != NULL) {
        for (size_t j = 0; j < strlen(msg); ++j) {
            uint8_t byte = (uint8_t)msg[j];
            checksum += byte;
            payload[i++] = byte;
        }
    }
    payload[i++] = checksum;
    payload[i++] = (uint8_t)CR;
    return i;
}

/*
 * parsing of the message in the payload
 * @param payload payload
 * @param msg buffer of char where to save the message
 * @param size number of bytes in the payload
 */
void parse_payload(const uint8_t *payload, char *msg, const size_t size) {
    if (msg != NULL) {
        for(size_t i = MSGBOUND; i < size - MSGBOUND; ++i)
            msg[i - MSGBOUND] = (char)payload[i];
    }
}

/*
 * check the payload
 * @param payload payload
 * @param expected_code control character expected
 * @param size number of bytes in the payload
 * @return true if the payload is valid, false otherwise
 */
bool is_valid(const uint8_t *payload, const uint8_t expected_code, const size_t size) {
    size_t i = 0;
    if (payload[i++] != (uint8_t)SOH || 
        payload[i++] != expected_code ||
        payload[size - 1] != (uint8_t)CR
    )
        return false;
    
    uint8_t checksum = payload[0] + payload[1];
    while (payload[i+1] != (uint8_t)CR) {
        checksum += payload[i++];
    }

    return checksum == payload[i];
}
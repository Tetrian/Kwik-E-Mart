
import socket as sk
import helper.payload as pl

import logging
logger = logging.getLogger('root')


HOST = 'localhost'
PORT = 8080

class Client():
    def __init__(self):
        try:
            self.socket = sk.socket(sk.AF_INET, sk.SOCK_STREAM)
            self.socket.connect((HOST, PORT))
        except sk.error as err:
            logger.error(f"Socket error! Cause: {err}")
            raise ValueError("can't connect client to server")
        logger.info('Client Ready!')
    
    # write a message to the socket
    def write_msg(self, code, msg):
        payload = pl.create_payload(code, msg)
        sent = self.socket.send(payload)
        if sent == 0:
            raise RuntimeError("socket connection broken")
    
    # read a message from the socket
    def read_msg(self, expected_code):
        payload = self.socket.recv(pl.BUFFSIZE)
        if payload == b'':
            raise RuntimeError("socket connection broken")
        if not pl.is_valid(payload, expected_code, len(payload)):
            self.socket.write(pl.NAK.to_bytes(1, 'little'))
            msg = None
        else:
            self.socket.write(pl.ACK.to_bytes(1, 'little'))
            msg = pl.parse_payload(payload, len(payload))
        return msg


if __name__ == '__main__':
    import logging.config
    logging.config.fileConfig('helper/logging.conf')

    client = Client()

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
    def write_msg(self, code, msg=None):
        payload = pl.create_payload(code, msg)
        sent = self.socket.send(payload)
        if sent == 0:
            raise RuntimeError("socket connection broken")
    
    # read a message from the socket
    def read_msg(self, expected_code):
        payload = self.socket.recv(pl.BUFFSIZE)
        if payload == b'':
            raise RuntimeError("socket connection broken")
        if pl.is_valid(payload, expected_code):
            msg = pl.parse_payload(payload)
        else:
            msg = None
        return msg


# Test
if __name__ == '__main__':
    import logging.config
    logging.config.fileConfig('helper/logging.conf')

    client = Client()
    client.write_msg(pl.BEL)
    msg = client.read_msg(pl.BEL)
    logger.info(msg)
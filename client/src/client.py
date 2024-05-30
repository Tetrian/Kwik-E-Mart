
import socket as sk

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
            raise ValueError
        logger.info('Client Ready!')



if __name__ == '__main__':
    import logging.config
    logging.config.fileConfig('helper/logging.conf')

    client = Client()
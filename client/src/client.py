
import socket as sk
import helper.payload as pl
from helper.parser import parse_products as parse
from random import randint, uniform

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
            logger.error(f"Socket error! Cause {err}")
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

    try:
        client = Client()
    except ValueError as err:
        logger.error(f'Connection to server refuse. Cause: {err}')
        exit()

    try:
        client.write_msg(pl.BEL)
        msg_products = client.read_msg(pl.BEL)
    except RuntimeError as err:
        logger.error(f'Exit cause: {err}')
        exit()

    products = dict(parse(msg_products))
    logger.debug(f'Products in the shop:\n\t{products}')
        
    n_products = randint(0, 10)
    total_price = round(uniform(10, 1000), 2)
    logger.info(f'Get in line for the checkout with {n_products} products')
    si_msg = str(n_products) + '$' + str(total_price)
    logger.debug(f'SI args: "{si_msg}"')
    
    try:
        client.write_msg(pl.SI, si_msg)
    except RuntimeError as err:
        logger.error(f'Exit cause: {err}')
        exit()
        
    logger.info('Waiting for server repsonse')
    try:
        response = client.read_msg(pl.SO)
    except RuntimeError as err:
        logger.error(f'Exit cause: {err}')
    logger.debug(response)
    logger.info('Client exit from store.')

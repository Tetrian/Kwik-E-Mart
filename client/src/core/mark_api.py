
import logging
logger = logging.getLogger('root')

def parse_products(string):
    products = string.split('$')

    dct = dict()
    for product in products:
        name, price = product.split('€')
        dct[name] = price
    
    logger.info('Parsing of products done.')
    return dct


# Test
if __name__ == '__main__':
    import logging.config
    logging.config.fileConfig('../helper/logging.conf')

    dictionary = parse_products(
        "Duff Beer€84.02$KrustyO's€39.44$Pizza€78.31$Donuts€79.84$Cola€91.16"
    )
    logger.info(f"Dictionary\n{dictionary}")
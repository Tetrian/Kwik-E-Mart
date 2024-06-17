
import logging
logger = logging.getLogger('root')

def parse_products(string):
    products = string[:-1].split('$')

    dct = dict()
    for product in products:
        name, price = product.split('€')
        dct[name] = float(price)
    
    logger.info('Parsing of products done.')
    return dct

def get_resource(name):
    res = ''
    match name:
        case 'Duff Beer':
            res = 'duff.png'
        case 'KrustyO\'s':
            res = 'cereal.jpg'
        case 'Pizza':
            res = 'pizza.jpg'
        case 'Donut':
            res = 'donut.png'
        case 'Buzz Cola':
            res = 'cola.jpg'
        case 'Slurpee':
            res = 'slurpee.jpg'
        case 'Chicken':
            res = 'chicken.jpg'
    return res


# Test
if __name__ == '__main__':
    import logging.config
    logging.config.fileConfig('logging.conf')

    dictionary = parse_products(
        "Duff Beer€84.02$KrustyO's€39.44$Pizza€78.31$Donut€79.84$Cola€91.16$"
    )
    logger.info(f"Dictionary\n{dictionary}")

    for name in dictionary.keys():
        logger.info(f'Name:{name}\tSource: {get_resource(name)}')
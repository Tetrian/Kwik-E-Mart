
from kivy.app import App
from kivy.uix.screenmanager import ScreenManager, Screen
from kivy.properties import ListProperty
from kivy.clock import Clock

from client import Client
from helper.parser import parse_products, get_resource
from helper.payload import BEL, SI, SO
from core.product import Product

import logging
logger = logging.getLogger('root')

# Manage the entrance of the shop
class OutsideScreen(Screen):
    def __init__(self, **kwargs):
        super(OutsideScreen, self).__init__(**kwargs)
        Clock.schedule_once(self.enter_request)
        self.event = Clock.schedule_interval(self.on_entrance, 3)
    
    # Send to server the request for enter in the shop
    def enter_request(self, dt):
        logger.info("Send request for enter in the market.")
        app = App.get_running_app()
        try:
            app.client.write_msg(BEL, '')
        except RuntimeError as err:
            logger.error(err)
            app.stop()

    # Read the server response
    def on_entrance(self, dt):
        logger.info("Try to enter in the market.")
        app = App.get_running_app()
        try:
            msg = app.client.read_msg(BEL)
        except RuntimeError as err:
            logger.error(err)
            app.stop()
        if msg == None:
            logger.info("Permission denied.")
        else:
            self.event.cancel()
            app.on_inside(msg)


# Manage the shopping
class InsideScreen(Screen):
    cart = ListProperty([])

    def __init__(self, **kwargs):
        super(InsideScreen, self).__init__(**kwargs)
        self.event = Clock.schedule_interval(self.add_products, 1)
    
    # wait for the change of the screen
    def add_products(self, dt):
        app = App.get_running_app()
        if app.manager.current == 'inside':
            self.init_shelf(app)
            self.event.cancel()

    # add product to shelf
    def init_shelf(self, app):
        shelf = self.ids.shelf
        for name, price in app.products.items():
            source = get_resource(name)
            product = Product(name, price, source)
            shelf.add_widget(product)
    
    # print the cart when list change
    def on_cart(self, instance, lst):
        logger.info(f'Current Cart is {[p[0] for p in lst]}')
    
    # queue up at the checkout
    def on_pay_btn(self):
        logger.info('Get in line for the checkout')
        
        total = 0
        for product in self.cart:
            total += product[1]
        
        msg = str(len(self.cart)) + '$' + str(total)

        app = App.get_running_app()
        try:
            app.client.write_msg(SI, msg)
        except RuntimeError as err:
            logger.error(err)
            app.stop()
        app.on_checkout(total)


# Manage the checkout
class CheckoutScreen(Screen):
    def  __init__(self, **kwargs):
        super(CheckoutScreen, self).__init__(**kwargs)
        self.event = Clock.schedule_interval(self.on_wait, 1)
    
    # wait for the change of the screen
    def on_wait(self, dt):
        app = App.get_running_app()
        if app.total != -1:
            self.event.cancel()
            self.on_pay()
    
    # manage the exit from the shop
    def on_pay(self):
        app = App.get_running_app()
        try:
            app.client.read_msg(SO)
        except RuntimeError as err:
            logger.error(err)
            app.stop()
        if (app.total > 0):
            logger.info(f'Payed €{app.total}')
        logger.info('Exit from the shop.')
        app.stop()


# MAIN APPLICATION
class Kwik_E_MartApp(App):
    def __init__(self, **kwargs):
        super(Kwik_E_MartApp, self).__init__(**kwargs)
        self.client = Client()
        self.products = dict()
        self.total = -1

    def build(self):
        sm = ScreenManager()
        sm.add_widget(OutsideScreen(name='outside'))
        sm.add_widget(InsideScreen(name='inside'))
        sm.add_widget(CheckoutScreen(name='checkout'))
        self.manager = sm
        return sm
    
    def on_inside(self, string):
        self.products = dict(parse_products(string))
        self.manager.current = 'inside'
    
    def on_checkout(self, total):
        self.total = total
        self.manager.current = 'checkout'
    
    def on_stop(self):
        logger.info("Application is shutting down.")


if __name__ == '__main__':
    try:
        Kwik_E_MartApp().run()
    except ValueError as err:
        logger.error(f'Exit. Cause {err}')
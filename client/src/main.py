
from kivy.app import App
from kivy.uix.screenmanager import ScreenManager, Screen
from kivy.properties import ListProperty
from kivy.clock import Clock

from client import Client
from helper.parser import parse_products, get_resource
from helper.payload import BEL
from core.product import Product

import logging
logger = logging.getLogger('root')

class OutsideScreen(Screen):
    def __init__(self, **kwargs):
        super(OutsideScreen, self).__init__(**kwargs)
        Clock.schedule_once(self.enter_request)
        self.event = Clock.schedule_interval(self.on_entrance, 3)
    
    def enter_request(self, dt):
        logger.info("Send request for enter in the market.")
        app = App.get_running_app()
        app.client.write_msg(BEL, '')

    def on_entrance(self, dt):
        logger.info("Try to enter in the market.")
        app = App.get_running_app()
        msg = app.client.read_msg(BEL)
        if msg == None:
            logger.info("Permission denied.")
        else:
            self.event.cancel()
            app.on_inside(msg)


class InsideScreen(Screen):
    cart = ListProperty([])

    def __init__(self, **kwargs):
        super(InsideScreen, self).__init__(**kwargs)
        self.event = Clock.schedule_interval(self.add_products, 1)
    
    def add_products(self, dt):
        app = App.get_running_app()
        if app.manager.current == 'inside':
            self.init_shelf(app)
            #logger.debug(app.products)
            self.event.cancel()

    def init_shelf(self, app):
        shelf = self.ids.shelf
        for name, price in app.products.items():
            source = get_resource(name)
            product = Product(name, price, source)
            shelf.add_widget(product)
    
    def on_cart(self, instance, lst):
        logger.info(f'Current Cart is {[p[0] for p in lst]}')

class Kwik_E_MartApp(App):
    def __init__(self, **kwargs):
        super(Kwik_E_MartApp, self).__init__(**kwargs)
        self.client = Client()
        self.products = dict()

    def build(self):
        sm = ScreenManager()
        sm.add_widget(OutsideScreen(name='outside'))
        sm.add_widget(InsideScreen(name='inside'))
        self.manager = sm
        return sm
    
    def on_inside(self, string):
        self.products = dict(parse_products(string))
        self.manager.current = 'inside'


if __name__ == '__main__':
    Kwik_E_MartApp().run()
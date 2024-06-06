
from kivy.app import App
from kivy.uix.label import Label
from kivy.uix.screenmanager import ScreenManager, Screen
from kivy.clock import Clock

from client import Client
import helper.payload as pl

import logging
logger = logging.getLogger('root')

class OutsideScreen(Screen):
    def __init__(self, **kwargs):
        super(Screen, self).__init__(**kwargs)
        self.event = Clock.schedule_interval(self.on_entrance, 3)
    
    def on_entrance(self, dt):
        logger.info("Try to enter in the market.")
        app = App.get_running_app()
        msg = app.client.read_msg(pl.BEL)
        if msg == None:
            logger.info("Permission denied.")
        else:
            self.event.cancel()
            app.on_inside(msg)
            


class InsideScreen(Screen):
    pass


class Kwik_E_MartApp(App):
    def __init__(self, **kwargs):
        super().__init__(**kwargs)
        self.client = Client()

    def build(self):
        sm = ScreenManager()
        sm.add_widget(OutsideScreen(name='outside'))
        sm.add_widget(InsideScreen(name='inside'))
        self.manager = sm
        return sm
    
    def on_inside(self, lst):
        #TODO:parsing products list
        logger.debug(lst)
        self.manager.current = 'inside'


if __name__ == '__main__':
    import logging.config
    logging.config.fileConfig('helper/logging.conf')
    Kwik_E_MartApp().run()
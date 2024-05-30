
from kivy.app import App
from kivy.uix.label import Label
from kivy.uix.screenmanager import ScreenManager, Screen

from client import Client

import logging
logger = logging.getLogger('root')

class OutsideScreen(Screen):
    pass


class Kwik_E_MartApp(App):
    def __init__(self, **kwargs):
        super().__init__(**kwargs)
        self.client = Client()

    def build(self):
        sm = ScreenManager()
        sm.add_widget(OutsideScreen(name='outside'))
        return sm


if __name__ == '__main__':
    import logging.config
    logging.config.fileConfig('helper/logging.conf')
    Kwik_E_MartApp().run()
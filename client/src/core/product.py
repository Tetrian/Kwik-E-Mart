
from kivy.uix.boxlayout import BoxLayout
from kivy.lang.builder import Builder

Builder.load_string('''
<Product>:
    orientation: 'vertical'
    size_hint: None, None
    name: 'ProductName'
    price: 0.0
    source: '../res/sign.png'
    
    Image:
        pos: self.pos
        size: self.size
        source: self.parent.source
    
    Label:
        text: self.parent.name
        size_hint: 1, None
    
    Label:
        text: '€ ' + str(self.parent.price)
        size_hint: 1, None
''')

class Product(BoxLayout):
    def __init__(self, name='ProductName', price=0.0, source='', **kwargs):
        super(Product, self).__init__(**kwargs)
        self.name = name
        self.price = price
        if source != '':
            self.source = '../res/products/' + source
    
    # When the touch has occured inside the widgets area
    # add this product to cart
    def on_touch_down(self, touch):
        if self.collide_point(*touch.pos):
            app = App.get_running_app()
            app.cart.append((self.name, self.price))


if __name__ == '__main__':
    from kivy.app import App

    class ProductApp(App):
        cart = []

        def build(self):
            bl = BoxLayout()
            bl.add_widget(Product('1', 1))
            bl.add_widget(Product('Duff Beer', 4.44, 'duff.png'))
            bl.bind(on_touch_up = lambda x, y: print(self.cart))
            return bl
    
    ProductApp().run()


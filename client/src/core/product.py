
from kivy.uix.boxlayout import BoxLayout
from kivy.lang.builder import Builder

Builder.load_string('''
<Product>:
    orientation: 'vertical'
    name: 'ProductName'
    price: 0.0
    source: '../res/sign.png'
    
    canvas.before:
        Color:
            rgba: 1, 1, 1, .3
        Rectangle:
            pos: self.pos
            size: self.size
        Color:
            rgba: .5, .5, .5, 1
        Line:
            width: 2
            rectangle: self.x, self.y, self.width, self.height
    
    Image:
        pos: self.pos
        size: self.size
        source: self.parent.source
    
    Label:
        text: self.parent.name
        size_hint: 1, .2
    
    Label:
        text: '€ ' + str(self.parent.price)
        size_hint: 1, .2
''')


class Product(BoxLayout):
    def __init__(self, name='ProductName', price=0.0, source='', **kwargs):
        super(Product, self).__init__(**kwargs)
        self.name = name
        self.price = price
        if source != '':
            self.source = '../res/products/' + source
    
    # When the touch has occured inside the widgets area
    # add this product to list property of InsideScreen
    def on_touch_down(self, touch):
        if self.collide_point(*touch.pos):
            screen = self.parent.parent.parent
            screen.cart.append((self.name, self.price))


# Test
if __name__ == '__main__':
    from kivy.app import App

    class ProductApp(App):
        def build(self):
            bl = BoxLayout()
            bl.add_widget(Product('1', 1))
            bl.add_widget(Product('Duff Beer', 4.44, 'duff.png'))
            return bl
    
    ProductApp().run()


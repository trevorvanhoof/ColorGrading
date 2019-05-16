from math import log
from PyQt4.QtCore import *
from PyQt4.QtGui import *


class ColorWheelWidget(QWidget):
    changed = pyqtSignal()

    def __init__(self):
        super(ColorWheelWidget, self).__init__()
        # the Y control (alt drag X) works as an additive to RGB
        self._y = 0.0

        # the main circle control just moves red and blue between -1 and 1
        # TODO: this is a little more complicated when looking at
        #  professional grading tools, but I couldn't find any reference
        #  on what the logic is behind the track ball control.
        self._red = 0.0
        self._green = 0.0  # green is never actaully controlled
        self._blue = 0.0

        # the outer ring control (ctrl drag X) mixes all values towards -1 or 1
        self._white = 0.0

        # the white slider starts at the top by default, but e.g. the gain subclass
        # will make it start at the bottom
        self._arcStart = 90

        self.motionRegion = QRect()

        # allow sub classes to override default values without reimplementing them in the constructor
        self.reset()

        self.setFixedSize(128, 128)
        self.changed.connect(self.repaint)

    @property
    def y(self):
        return self._y

    @property
    def red(self):
        return self._red

    @property
    def green(self):
        return self._green

    @property
    def blue(self):
        return self._blue

    @property
    def white(self):
        return self._white

    """
    @y.setter
    def y(self, y):
        self._y = y

    @red.setter
    def red(self, red):
        self._red = red

    @green.setter
    def green(self, green):
        self._green = green

    @blue.setter
    def blue(self, blue):
        self._blue = blue

    @white.setter
    def blue(self, white):
        self._white = white
    """

    def reset(self):
        self._y = 0.0
        self._red = 0.0
        self._green = 0.0
        self._blue = 0.0
        self._white = 0.0
        self.changed.emit()

    def mousePressEvent(self, event):
        self._dragInfo = event.modifiers(), event.x(), event.y(), self._y, self._red, self._green, self._blue, self._white

    def mouseMoveEvent(self, event):
        dx = (event.x() - self._dragInfo[1]) / float(self.motionRegion.width())
        dy = (event.y() - self._dragInfo[2]) / float(self.motionRegion.height())

        # hold alt for Y control
        if self._dragInfo[0] & Qt.AltModifier:
            self._y = self._dragInfo[3] + dx * 0.1
            self.changed.emit()
            return

        # hold ctrl for white control
        if self._dragInfo[0] & Qt.ControlModifier:
            self._white = self._dragInfo[7] + dx * 0.02
            self.changed.emit()
            return

        # hold shift for direct control
        # else we are way more sensitive
        if self._dragInfo[0] & Qt.ShiftModifier:
            self._blue = ((event.x() - self.motionRegion.x()) / float(self.motionRegion.width())) * 2.0 - 1.0
            self._red = ((event.y() - self.motionRegion.y()) / float(self.motionRegion.height())) * 2.0 - 1.0

        self._blue = self._dragInfo[6] + dx * 0.1
        self._red = self._dragInfo[4] - dy * 0.1

        self.changed.emit()

    def paintEvent(self, event):
        # palette
        mid = QColor(80, 80, 80)
        dark = QColor(40, 40, 40)
        light = QColor(140, 140, 140)

        painter = QPainter(self)
        painter.setPen(Qt.NoPen)
        painter.setRenderHints(QPainter.Antialiasing | QPainter.HighQualityAntialiasing)

        r = QRect(0, 0, self.width(), self.height())

        # draw the background
        painter.fillRect(r, mid)

        # draw the opacity slider background
        painter.setBrush(dark)
        r.adjust(4, 4, -4, -4)
        painter.drawEllipse(r)

        # draw the opacity slider value
        painter.setBrush(light)
        wedge = QPainterPath()
        wedge.moveTo(r.center())
        wedge.arcTo(QRectF(r), self._arcStart, -min(1, max(-1, self._white)) * 180)
        painter.drawPath(wedge)

        # draw the inset to mask the opacity slider
        r.adjust(2, 2, -2, -2)
        painter.setBrush(mid)
        painter.drawEllipse(r)

        # draw the hue gradient
        r.adjust(6, 6, -6, -6)
        self.motionRegion = QRect(r)
        gradient = QConicalGradient()

        def addColor(n, cd):
            gradient.setColorAt(n / 6.0, cd)
            if n == 0:
                gradient.setColorAt(1.0, cd)
            return (n + 1) % 6

        n = 2
        n = addColor(n, QColor(255, 0, 0))
        n = addColor(n, QColor(255, 255, 0))
        n = addColor(n, QColor(0, 255, 0))
        n = addColor(n, QColor(0, 255, 255))
        n = addColor(n, QColor(0, 0, 255))
        n = addColor(n, QColor(255, 0, 255))
        gradient.setCoordinateMode(QGradient.ObjectBoundingMode)
        gradient.setCenter(0.5, 0.5)
        painter.setBrush(gradient)
        painter.drawEllipse(r)

        # draw a drop shadow
        r.adjust(8, 8, -8, -8)
        painter.setBrush(dark)
        painter.drawEllipse(r)

        # draw the center widget region
        r.adjust(2, 2, -2, -2)
        painter.setBrush(mid)
        painter.drawEllipse(r)

        # draw the axes
        painter.setBrush(Qt.NoBrush)
        painter.setPen(light)
        x0, y0, x1, y1 = r.x(), r.y(), r.right(), r.bottom()
        x, y = r.center().x(), r.center().y()
        x += .5
        y += .5
        painter.drawLine(QLineF(x0, y, x1, y))
        painter.drawLine(QLineF(x, y0, x, y1))

        # draw the track value
        magnitude = max((self._blue ** 2 + self._red ** 2) ** 0.5, 1.0)
        if magnitude:
            x = self._blue / magnitude
            y = -self._red / magnitude
        else:
            x = 0
            y = 0
        x *= self.motionRegion.width() * 0.5
        y *= self.motionRegion.height() * 0.5

        cursor = self.motionRegion.center() + QPointF(int(x), int(y))
        m = 11
        r = QRectF(cursor - QPointF(m / 2, m / 2), QSizeF(m, m))
        pen = QPen(light)
        pen.setWidth(2.5)
        painter.setPen(pen)
        painter.drawEllipse(r)


class GainWidget(ColorWheelWidget):
    def __init__(self):
        super(GainWidget, self).__init__()

    @property
    def white(self):
        if self._white <= 0.0:
            return self._white
        else:
            return self._white * 15.0

    @property
    def y(self):
        if self._y <= 0.0:
            return self._y
        else:
            return self._y * 15.0

    @property
    def red(self):
        if self._red <= 0.0:
            return self._red
        else:
            return self._red * 2.0

    @property
    def green(self):
        if self._green <= 0.0:
            return self._green
        else:
            return self._green * 2.0

    @property
    def blue(self):
        if self._blue <= 0.0:
            return self._blue
        else:
            return self._blue * 2.0


class ColorWheel(QWidget):
    @classmethod
    def wheelClass(cls):
        return ColorWheelWidget

    def __init__(self, caption):
        super(ColorWheel, self).__init__()
        hbar = QHBoxLayout()

        self._y = QLabel('0.00\nY')
        self._y.setAlignment(Qt.AlignCenter)
        hbar.addWidget(self._y)
        self._y.setFixedWidth(32)

        self._red = QLabel('0.00\nR')
        self._red.setAlignment(Qt.AlignCenter)
        hbar.addWidget(self._red)
        self._red.setFixedWidth(32)

        self._green = QLabel('0.00\nG')
        self._green.setAlignment(Qt.AlignCenter)
        hbar.addWidget(self._green)
        self._green.setFixedWidth(32)

        self._blue = QLabel('0.00\nB')
        self._blue.setAlignment(Qt.AlignCenter)
        hbar.addWidget(self._blue)
        self._blue.setFixedWidth(32)

        self.wheel = self.wheelClass()()
        self.wheel.changed.connect(self.sync)

        title = QHBoxLayout()
        captionLabel = QLabel(caption)
        title.addWidget(captionLabel)
        captionLabel.setAlignment(Qt.AlignCenter)
        reset = QPushButton('Reset')
        reset.clicked.connect(self.wheel.reset)
        reset.setIconSize(QSize(24, 24))
        reset.setFixedSize(24, 24)
        title.addWidget(reset)
        title.setStretch(0, 1)

        vbar = QVBoxLayout()
        self.setLayout(vbar)
        vbar.addLayout(title)
        vbar.addWidget(self.wheel)
        vbar.addLayout(hbar)

        self.changed = self.wheel.changed
        self.sync()

    def sync(self):
        self._y.setText('%.02f\nY' % (self.wheel.y + self.wheel.white))
        self._red.setText('%.02f\nR' % (self.wheel.red + self.wheel.white))
        self._green.setText('%.02f\nG' % (self.wheel.green + self.wheel.white))
        self._blue.setText('%.02f\nB' % (self.wheel.blue + self.wheel.white))


class GainWheel(ColorWheel):
    @classmethod
    def wheelClass(cls):
        return GainWidget


class OffsetWheel(ColorWheel):
    def __init__(self, caption):
        super(OffsetWheel, self).__init__(caption)
        self._y.hide()


class LabelSlider(QLabel):
    valueChanged = pyqtSignal(float)

    def __init__(self, caption):
        super(LabelSlider, self).__init__(caption)
        self._caption = caption
        self._minimum = 0.0
        self._maximum = 1.0
        self._value = 0.0
        self.valueChanged.connect(self.repaint)

    def background(self):
        return None

    def foreground(self):
        return QBrush(QColor(120, 160, 220))

    def value(self):
        return self._value

    def setValue(self, value):
        self._value = min(max(value, self._minimum), self._maximum)
        self.setText('%s: %.02f' % (self._caption, self._value))
        self.valueChanged.emit(self._value)

    def mouseMoveEvent(self, event):
        t = event.x() / float(self.width())
        self.setValue(t * (self._maximum - self._minimum) + self._minimum)

    def paintEvent(self, event):
        painter = QPainter(self)
        geo = QRect(0, 0, self.width(), self.height())

        bg = self.background()
        if bg is not None:
            painter.fillRect(geo, bg)

        fg = self.foreground()
        if fg is not None:
            geo.setWidth(int(geo.width() * ((self._value - self._minimum) / (self._maximum - self._minimum))))
            painter.fillRect(geo, fg)

        super(LabelSlider, self).paintEvent(event)


def colorFromKelvin(temperature):
    # from http://www.tannerhelland.com/4435/convert-temperature-rgb-algorithm-code/
    # photographic temperature values are between 15 to 150
    if temperature <= 66.0:
        g = min(max(int((99.4708025861 * log(temperature) - 161.1195681661)), 0), 255)
        if temperature >= 19.0:
            b = min(max(int(138.5177312231 * log(temperature - 10.0) - 305.0447927307), 0), 255)
            return QColor(255, g, b)
        return QColor(255, g, 0)
    r = min(max(int(329.698727446 * pow(temperature - 60.0, -0.1332047592)), 0), 255)
    g = min(max(int(288.1221695283 * pow(temperature - 60.0, -0.0755148492)), 0), 255)
    return QColor(r, g, 255)


class TemperatureSlider(LabelSlider):
    def value(self):
        return self.mapped(self._value)

    def _setInternalValue(self, value):
        value = min(1.0, max(0.0, value))
        self._value = value
        value = self.mapped(value)
        self.setText('%s: %.02f' % (self._caption, value))
        self.valueChanged.emit(value)

    def setValue(self, value):
        self._value = self.unmapped(value)
        self.setText('%s: %.02f' % (self._caption, value))
        self.valueChanged.emit(value)

    def mouseMoveEvent(self, event):
        t = min(1.0, max(0.0, event.x() / float(self.width())))
        self._value = t
        value = self.mapped(t)
        self.setText('%s: %.02f' % (self._caption, value))
        self.valueChanged.emit(value)

    def mapped(self, t):
        t -= 0.5
        s = -0.5 if t < 0.0 else 0.5
        t = pow(abs(t + t), 2.0) * s + 0.5
        return pow(t, 6.0) * (4000.0 - 6.0) + 6.0

    def unmapped(self, t):
        t = pow((t - 7.0) / 4000.0, 1.0 / 6.0)
        t -= 0.5
        s = -0.5 if t < 0.0 else 0.5
        return pow(abs(t + t), 1.0 / 2.0) * s + 0.5

    def foreground(self):
        g = QLinearGradient()
        g.setFinalStop(1.0, 0.0)
        g.setCoordinateMode(QGradient.ObjectBoundingMode)
        for i in xrange(11):
            temperature = self.mapped(self._value * i * 0.1)
            g.setColorAt(i * 0.1, colorFromKelvin(temperature))
        return g

    def background(self):
        return colorFromKelvin(self.value())


class ColorCorrect(QWidget):
    changed = pyqtSignal()

    def __init__(self):
        super(ColorCorrect, self).__init__()

        main = QHBoxLayout()
        self.setLayout(main)

        self.lift = ColorWheel('Lift')
        main.addWidget(self.lift)
        self.gamma = ColorWheel('Gamma')
        main.addWidget(self.gamma)
        self.gain = GainWheel('Gain')
        main.addWidget(self.gain)
        self.offset = OffsetWheel('Offset')
        main.addWidget(self.offset)

        self.lift.changed.connect(self.changed.emit)
        self.gamma.changed.connect(self.changed.emit)
        self.gain.changed.connect(self.changed.emit)
        self.offset.changed.connect(self.changed.emit)

        # by lack of more info I'm using temperature as a while balance input
        values = QGridLayout()
        main.addLayout(values)

        def slider(row, label, minimum, maximum, value):
            values.addWidget(QLabel(label), row, 0)
            inst = QDoubleSpinBox()
            inst.setSingleStep(0.01)
            inst.setDecimals(3)
            inst.setMinimum(minimum)
            inst.setMaximum(maximum)
            inst.setValue(value)
            inst.valueChanged.connect(self.changed.emit)
            values.addWidget(inst, row, 1)
            return inst, row + 1

        r = 0
        self.contrast, r = slider(r, 'Contrast', 0.0, 2.0, 1.0)
        self.contrastPivot, r = slider(r, 'Pivot', 0.0, 1.0, 0.435)
        self.saturation, r = slider(r, 'Saturation', 0.0, 2.0, 1.0)
        self.hue, r = slider(r, 'Hue shift', -6.0, 6.0, 0.0)

        # value in kelvin used to derive rgb to tint with
        self.temperature, r = slider(4, 'Temperature', 0.0, 4000.0, 66.0)
        # like temperature but mixing green and magenta
        # self.tint, r = slider(r, 'Tint', -100.0, 100.0)

        # unsharp mask, can go negative to fade in blur instead
        self.unsharpMask, r = slider(r, 'Unsharp mask', -1.0, 1.0, 0.0)
        # self.vibrance, r = slider(r, 'Vibrance', -100.0, 100.0)
        # this seems to raise to 1.0 / value but the response is only in the shadows
        # self.shadows, r = slider(r, 'Shadows', -100.0, 100.0)
        # this seems to mutiply based one existing brightness
        # color += color * highlights
        # self.highlights, r = slider(5, 'Highlights', -100.0, 100.0)

    def grading(self):
        def bwfade(a, w):
            if w < 0.0:
                b = -1.0
                w = -w
            else:
                b = 1.0
            return a + (b - a) * w

        gradingUniforms = {
            'uLift'         : [bwfade(self.lift.wheel.red + self.lift.wheel.y, self.lift.wheel.white),
                               bwfade(self.lift.wheel.green + self.lift.wheel.y, self.lift.wheel.white),
                               bwfade(self.lift.wheel.blue + self.lift.wheel.y, self.lift.wheel.white)],
            'uGamma'        : [bwfade(self.gamma.wheel.red + self.gamma.wheel.y, self.gamma.wheel.white),
                               bwfade(self.gamma.wheel.green + self.gamma.wheel.y, self.gamma.wheel.white),
                               bwfade(self.gamma.wheel.blue + self.gamma.wheel.y, self.gamma.wheel.white)],
            'uGain'         : [bwfade(self.gain.wheel.red + self.gain.wheel.y, self.gain.wheel.white),
                               bwfade(self.gain.wheel.green + self.gain.wheel.y, self.gain.wheel.white),
                               bwfade(self.gain.wheel.blue + self.gain.wheel.y, self.gain.wheel.white)],
            'uOffset'       : [bwfade(self.offset.wheel.red, self.offset.wheel.white),
                               bwfade(self.offset.wheel.green, self.offset.wheel.white),
                               bwfade(self.offset.wheel.blue, self.offset.wheel.white)],
            'uContrast'     : self.contrast.value(),
            'uContrastPivot': self.contrastPivot.value(),
            'uSaturation'   : self.saturation.value(),
            'uHue'          : self.hue.value(),
            'uTemperature'  : self.temperature.value(),
            'uUnsharpMask'  : self.unsharpMask.value(),
        }

        return gradingUniforms


if __name__ == '__main__':
    a = QApplication([])
    cc = TemperatureSlider('caption')
    cc.show()
    a.exec_()

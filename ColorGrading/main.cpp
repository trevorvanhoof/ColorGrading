#define IMPL_GL

#include "gl.h"
#include <windows.h>
#include <QtCore>
#include <QtGui>
#include <QtWidgets>
#include "bufferformats.h"
#include "buffers.h"
#include "materials.h"
#include "alerts.h"

struct ColorWheelSettings
{
	float y;
	float red;
	float green;
	float blue;
	float white;
};

float clamp(float value, float low, float high)
{
	if (value > high) return high;
	if (value < low) return low;
	return value;
}

int clamp(int value, int low, int high)
{
	if (value > high) return high;
	if (value < low) return low;
	return value;
}

float max(float lhs, float rhs)
{
	if (lhs > rhs) return lhs; 
	return rhs;
}

QConicalGradient hueGradient()
{
	QConicalGradient gradient;
	
	gradient.setColorAt(0.0 / 6.0, QColor(0, 0, 255));
	gradient.setColorAt(1.0 / 6.0, QColor(255, 0, 255));
	gradient.setColorAt(2.0 / 6.0, QColor(255, 0, 0));
	gradient.setColorAt(3.0 / 6.0, QColor(255, 255, 0));
	gradient.setColorAt(4.0 / 6.0, QColor(0, 255, 0));
	gradient.setColorAt(5.0 / 6.0, QColor(0, 255, 255));
	gradient.setColorAt(6.0 / 6.0, QColor(0, 0, 255));

	gradient.setCoordinateMode(QGradient::ObjectBoundingMode);
	gradient.setCenter(0.5, 0.5);

	return gradient;
}

/*
Color wheel control inspired by DaVinci Resolve
Differences:
	- Their 2D control seems to operate on a gamut,
	this version never changes green.
	- Their UI reacts to changes from other parameters,
	e.g. the 2D control gets shifted to the center if 
	we up the white (outer ring) control,
	this version is more straight forward.
Controls:
	The outer ring (white) can be moved with CTRL + left mouse click drag.
	In the UI it raises YRGB uniformly.
	
	The Y value can be moved with ALT + drag.
	
	The B and R values are mapped to X and Y respectively
	and can be moved with just Drag, or moved faster with SHIFT + drag.

	The final values are converted to RGB as follows:
	lerp(RGB + Y, white > 0 ? 1 : 0, abs(white))

	This was just decided by looking at the parade view of
	a gradient and seeing what the individual controls would do.
*/
class ColorWheelWidget : public QWidget
{
	Q_OBJECT;
	
protected:
	// value of the control
	ColorWheelSettings state;

	// set during paint, used to map mouse input and clamp the 2D control inside the UI
	QRect motionRegion; 

	// drag action info, cached on mouse press
	Qt::KeyboardModifiers dragModifiers;
	QPoint dragStart;
	ColorWheelSettings dragStartState;

	// makes the outer wheel control start with 0 at the top, here to be overridden
	virtual int arcStart() { return 90; } 
public:
	ColorWheelWidget()
	{
		// implement default values here
		reset();

		setFixedSize(128, 128);
#pragma warning(suppress: 4100)
		connect(this, &ColorWheelWidget::changed, this, [=](ColorWheelSettings state) { this->repaint(); });
	}

	void reset()
	{
		state.y = 0.0f;
		state.red = 0.0f;
		state.green = 0.0f;
		state.blue = 0.0f;
		state.white = 0.0f;
		emit changed(state);
	}

	virtual void mousePressEvent(QMouseEvent* event) override
	{
		dragModifiers = event->modifiers();
		dragStart = event->pos();
		dragStartState = state;
	}

	virtual void mouseMoveEvent(QMouseEvent* event) override
	{
		// mouse delta in unit space
		float dx = (event->x() - dragStart.x()) / (float)motionRegion.width();
		float dy = (event->y() - dragStart.y()) / (float)motionRegion.height();

		// hold ALT to control the Y parameter
		if (dragModifiers & Qt::AltModifier)
		{
			state.y = dragStartState.y + dx * ((dragModifiers & Qt::ShiftModifier) ? 1.0f : 0.1f);
			emit changed(state);
			return;
		}

		// hold CTRL to control the white parameter
		if (dragModifiers & Qt::ControlModifier)
		{
			state.white = dragStartState.white + dx * ((dragModifiers & Qt::ShiftModifier) ? 0.2f : 0.02f);
			emit changed(state);
			return;
		}

		// hold SHIFT for direct control
		if (dragModifiers & Qt::ShiftModifier)
		{
			float x = (event->x() - motionRegion.x()) / (float)motionRegion.width();
			float y = (event->y() - motionRegion.y()) / (float)motionRegion.height();
			state.blue = x * 2.0f - 1.0f;
			state.red = 1.0f - y * 2.0f;
			emit changed(state);
			return;
		}

		state.blue = dragStartState.blue + dx * 0.1f;
		state.red = dragStartState.red - dy * 0.1f;
		emit changed(state);
		return;
	}

#pragma warning(suppress: 4100)
	virtual void paintEvent(QPaintEvent* event) override
	{
		const QColor light(140, 140, 140);
		const QColor mid(80, 80, 80);
		const QColor dark(40, 40, 40);

		QPainter painter(this);
		painter.setPen(Qt::NoPen);
		painter.setRenderHints(QPainter::Antialiasing | QPainter::HighQualityAntialiasing);
		
		QRect geo(0, 0, width(), height());
		
		// background
		painter.fillRect(geo, mid);
		
		// ring slider background
		painter.setBrush(dark);
		geo.adjust(4,4,-4,-4);
		painter.drawEllipse(geo);
		
		// ring slider value
		painter.setBrush(light);
		QPainterPath wedge;
		wedge.moveTo(geo.center());
		wedge.arcTo((QRectF)geo, arcStart(), clamp(state.white, -1.0f, 1.0f) * -180);
		painter.drawPath(wedge);
		
		// mask the center, so it's not a pie chart but a ring slider
		painter.setBrush(mid);
		geo.adjust(2, 2, -2, -2);
		painter.drawEllipse(geo);
		
		// draw the hue ring
		painter.setBrush(hueGradient());
		geo.adjust(6,6,-6,-6);
		painter.drawEllipse(geo);
		motionRegion = QRect(geo);
		
		// draw drop shadow
		painter.setBrush(dark);
		geo.adjust(8,8,-8,-8);
		painter.drawEllipse(geo);
		
		// mask out the drop shadow
		painter.setBrush(mid);
		geo.adjust(2,2,-2,-2);
		painter.drawEllipse(geo);
		
		// draw the axes of the central widget region
        painter.setBrush(Qt::NoBrush);
        painter.setPen(light);
        float x = geo.center().x() + 0.5f;
        float y = geo.center().y() + 0.5f;
        painter.drawLine(QLineF(geo.x(), y, geo.right(), y));
        painter.drawLine(QLineF(x, geo.y(), x, geo.bottom()));
		
		// draw the blue & red control
		float excessMagnitude = sqrtf(max(state.blue * state.blue + state.red * state.red, 1.0f));
		x = ((state.blue / excessMagnitude) * 0.5f) * motionRegion.width();
		y = ((-state.red / excessMagnitude) * 0.5f) * motionRegion.height();
		
		QPoint cursor = motionRegion.center() + QPoint((int)x, (int)y);
		QPen pen(light);
		pen.setWidth(2);
		painter.setPen(pen);
		painter.drawEllipse(QRect(cursor - QPoint(5, 5), QSize(11, 11)));
	}

	virtual ColorWheelSettings value() { return state; }
	virtual void setValue(const ColorWheelSettings& newState) { state = newState; }

signals:
	void changed(ColorWheelSettings state);
};

class GainWidget : public ColorWheelWidget
{
	Q_OBJECT;

public:
	virtual ColorWheelSettings value() override
	{
		return
		{
			state.y * ((state.y <= 0.0f) ? 1.0f : 15.0f),
			state.red * ((state.red <= 0.0f) ? 1.0f : 2.0f),
			state.green * ((state.green <= 0.0f) ? 1.0f : 2.0f),
			state.blue * ((state.blue <= 0.0f) ? 1.0f : 2.0f),
			state.white * ((state.white <= 0.0f) ? 1.0f : 15.0f)
		};
	}

	virtual void setValue(const ColorWheelSettings& newState) override
	{
		state.y = newState.y / ((state.y <= 0.0f) ? 1.0f : 15.0f);
		state.red = newState.y / ((state.red <= 0.0f) ? 1.0f : 2.0f);
		state.green = newState.y / ((state.green <= 0.0f) ? 1.0f : 2.0f);
		state.blue = newState.y / ((state.blue <= 0.0f) ? 1.0f : 2.0f);
		state.white = newState.white / ((state.white <= 0.0f) ? 1.0f : 15.0f);
	}
};

QVBoxLayout* vlayout()
{
	QVBoxLayout* l = new QVBoxLayout;
	l->setSpacing(0);
	l->setContentsMargins(0, 0, 0, 0);
	return l;
}

QHBoxLayout* hlayout()
{
	QHBoxLayout* l = new QHBoxLayout;
	l->setSpacing(0);
	l->setContentsMargins(0, 0, 0, 0);
	return l;
}

class ColorWheel : public QWidget
{
	Q_OBJECT;

protected:
	QLabel* y;
	QLabel* red;
	QLabel* green;
	QLabel* blue;
	ColorWheelWidget* wheel;

	virtual void instantiateWheel() 
	{
		wheel = new ColorWheelWidget;
		connect(wheel, &ColorWheelWidget::changed, this, &ColorWheel::forwardChanged);
	}

	void forwardChanged(ColorWheelSettings state) { emit changed(state); sync(); }

public:
	ColorWheel(QString caption)
	{
		QVBoxLayout* vbar = vlayout();
		setLayout(vbar);

		QHBoxLayout* title = hlayout();
		vbar->addLayout(title);

		instantiateWheel();
		vbar->addWidget(wheel);

		QHBoxLayout* labels = hlayout();
		vbar->addLayout(labels);
		
		QLabel* captionLabel = new QLabel(caption);
		
		title->addWidget(captionLabel);
		QPushButton* reset = new QPushButton(QIcon("../reset.png"), "");
		title->addWidget(reset);
		title->setStretch(0, 1);

		captionLabel->setAlignment(Qt::AlignCenter);
		reset->setIconSize(QSize(24, 24));
		reset->setFixedSize(24, 24);
		connect(reset, &QPushButton::clicked, wheel, &ColorWheelWidget::reset);

		y = new QLabel;
		labels->addWidget(y);
		red = new QLabel;
		labels->addWidget(red);
		green = new QLabel;
		labels->addWidget(green);
		blue = new QLabel;
		labels->addWidget(blue);

		// initialize label text
		sync();

		y->setAlignment(Qt::AlignCenter);
		y->setFixedWidth(32);
		red->setAlignment(Qt::AlignCenter);
		red->setFixedWidth(32);
		green->setAlignment(Qt::AlignCenter);
		green->setFixedWidth(32);
		blue->setAlignment(Qt::AlignCenter);
		blue->setFixedWidth(32);
	}

	virtual ~ColorWheel()
	{
		delete wheel;
	}

	virtual void sync() 
	{
		ColorWheelSettings state = wheel->value();
		y->setText(format<QString>("%.02f\nY", state.y + state.white));
		red->setText(format<QString>("%.02f\nR", state.red + state.white));
		green->setText(format<QString>("%.02f\nG", state.green + state.white));
		blue->setText(format<QString>("%.02f\nB", state.blue + state.white));
	}

	QVector3D value()
	{
		ColorWheelSettings state = wheel->value();
		float r = state.red + state.y;
		r += state.white * (1.0f - r);
		float g = state.green + state.y;
		g += state.white * (1.0f - g);
		float b = state.blue + state.y;
		b += state.white * (1.0f - b);
		return QVector3D(r, g, b);
	}

signals:
	void changed(ColorWheelSettings state);
};

class GainWheel : public ColorWheel
{
	Q_OBJECT;

protected:
	virtual void instantiateWheel() override
	{
		wheel = new GainWidget;
	}

public:
	GainWheel() : ColorWheel("Gain") {}
};

class OffsetWheel : public ColorWheel
{
	Q_OBJECT;

public:
	OffsetWheel() : ColorWheel("Offset")
	{
		y->hide();
	}
};

class LabelSlider : public QLabel
{
	Q_OBJECT;

private:
	const QBrush fg = QBrush(QColor(100, 120, 160));

protected:
	float minimum = 0.0f;
	float maximum = 1.0f;
	float current = 0.0f;
	QString caption = "";

	virtual const QBrush* background() { return nullptr; }
	virtual const QBrush* foreground() { return &fg; }

public:
	explicit LabelSlider(const QString& text, float minimum = 0.0f, float maximum = 1.0f, float initialValue = 0.0f, QWidget *parent = Q_NULLPTR, Qt::WindowFlags f = Qt::WindowFlags()) :
		caption(text), minimum(minimum), maximum(maximum), QLabel("", parent, f)
	{
		setValue(initialValue);
		connect(this, SIGNAL(valueChanged(float)), this, SLOT(repaint()));
	}

	virtual float value()
	{
		return current;
	}

	virtual void setValue(float next)
	{
		current = clamp(next, minimum, maximum);
		CONVERT_QSTRING(caption, text);
		setText(format<QString>("%s: %.02f", text, next));
		emit valueChanged(current);
	}

	virtual void mouseMoveEvent(QMouseEvent* event) override
	{
		float t = event->x() / (float)width();
		setValue(t * (maximum - minimum) + minimum);
	}

	virtual void paintEvent(QPaintEvent* event) override
	{
		QPainter painter(this);
		QRect geo(0, 0, width(), height());
		
		const QBrush* bg = background();
		if(bg) 
			painter.fillRect(geo, *bg);
		
		const QBrush* fg = foreground();
		if (fg)
		{
			geo.setWidth(int(((current - minimum) / (maximum - minimum)) * geo.width()));
			painter.fillRect(geo, *fg);
		}

		// draw default label behaviour on top
		QLabel::paintEvent(event);
	}

signals:
	void valueChanged(float);
};

QColor colorFromKelvin(float temperature)
{
	// from http ://www.tannerhelland.com/4435/convert-temperature-rgb-algorithm-code/
	// photographic temperature values are between 15 to 150
	if (temperature <= 66.0f)
	{
		int g = clamp((int)((99.4708025861f * log(temperature) - 161.1195681661f)), 0, 255);
		if (temperature >= 19.0f)
		{
			int b = clamp((int)(138.5177312231 * log(temperature - 10.0) - 305.0447927307), 0, 255);
			return QColor(255, g, b);
		}
		return QColor(255, g, 0);
	}
	int r = clamp((int)(329.698727446f * pow(temperature - 60.0f, -0.1332047592f)), 0, 255);
	int g = clamp((int)(288.1221695283f * pow(temperature - 60.0f, -0.0755148492f)), 0, 255);
	return QColor(r, g, 255);
}

class TemperatureSlider : public LabelSlider
{
	Q_OBJECT;

protected:
	const float START = 6.0f;
	const float POWER = 6.0f;
	const float SMEAR = 3.0f;
	const float STOP = 4000.0f;

	float mapped(float t)
	{
		// do a kind of fish eye at 0.5 to stretch the interesting bit out more
		t -= 0.5f;
		float s = t < 0.0f ? -0.5f : 0.5f;
		t = powf(fabs(t + t), SMEAR) * s + 0.5f;
		// then push the main gradient to the center for maximum coverage
		t = pow(clamp(t, 0.0f, 1.0f), POWER) * (STOP - START) + START;
		return t;
	}

	float unmapped(float value)
	{
		float t = pow((value - START) / (STOP - START), 1.0f / POWER);
		t -= 0.5f;
		float s = t < 0.0f ? -0.5f : 0.5f;
		t = powf(fabs(t + t), 1.0 / SMEAR) * s + 0.5f;
		return t;
	}

public:
	explicit TemperatureSlider(const QString& text, QWidget *parent = Q_NULLPTR, Qt::WindowFlags f = Qt::WindowFlags()) :
		LabelSlider(text, 0.0f, 1.0f, 0.0f, parent, f)
	{
		setValue(66.0f);
	}

	virtual float value() override
	{
		return mapped(current);
	}

	virtual void setValue(float value) override
	{
		current = unmapped(value);
		CONVERT_QSTRING(caption, text);
		setText(format<QString>("%s: %.02f", text, value));
		emit valueChanged(value);
	}

	virtual void mouseMoveEvent(QMouseEvent* event) override
	{
		float t = clamp(event->x() / (float)width(), 0.0f, 1.0f);
		current = t;
		float value = mapped(t);
		CONVERT_QSTRING(caption, text);
		setText(format<QString>("%s: %.02f", text, value));
		emit valueChanged(value);
	}

	// approach the temperature gradient up until this point
	virtual QBrush* foreground() override 
	{
		QLinearGradient gradient;
		gradient.setFinalStop(1.0f, 0.0f);
		gradient.setCoordinateMode(QGradient::ObjectBoundingMode);
		const int N = 20;
		for (int i = 0; i <= N; ++i)
		{
			float temperature = mapped(current * i / (float)N);
			gradient.setColorAt(i / (float)N, colorFromKelvin(temperature));
		}
		return new QBrush(gradient);
	}

	// fill the remainder of the slider with the current color
	virtual QBrush* background() override 
	{
		return new QBrush(colorFromKelvin(mapped(current))); 
	}
};

// Utility to batch pass all color correction settings through a signal
struct GradingSettings
{
	QVector3D lift;
	QVector3D gamma;
	QVector3D gain;
	QVector3D offset;
	float contrast;
	float pivot;
	float saturation;
	float hueShift;
	float temperature;
	float unsharpMask;
};

class ColorCorrect : public QWidget
{
	Q_OBJECT;

	ColorWheel* lift;
	ColorWheel* gamma;
	GainWheel* gain;
	OffsetWheel* offset;
	LabelSlider* contrast;
	LabelSlider* pivot;
	LabelSlider* saturation;
	LabelSlider* hueShift;
	TemperatureSlider* temperature;
	LabelSlider* unsharpMask;

	void emitChanged()
	{
		emit changed(state());
	}

public:
	ColorCorrect()
	{
		QHBoxLayout* main = hlayout();
		setLayout(main);

		main->addWidget(lift = new ColorWheel("Lift"));
		main->addWidget(gamma = new ColorWheel("Gamma"));
		main->addWidget(gain = new GainWheel());
		main->addWidget(offset = new OffsetWheel());

		connect(lift, &ColorWheel::changed, this, &ColorCorrect::emitChanged);
		connect(gamma, &ColorWheel::changed, this, &ColorCorrect::emitChanged);
		connect(gain, &ColorWheel::changed, this, &ColorCorrect::emitChanged);
		connect(offset, &ColorWheel::changed, this, &ColorCorrect::emitChanged);

		QVBoxLayout * sliders = vlayout();
		main->addLayout(sliders);

		main->setStretch(4, 1);

		sliders->addWidget(contrast = new LabelSlider("Contrast", 0.0f, 2.0f, 1.0f));
		sliders->addWidget(pivot = new LabelSlider("Pivot", 0.0f, 1.0f, 0.435f));
		sliders->addWidget(saturation = new LabelSlider("Saturation", 0.0f, 2.0f, 1.0f));
		sliders->addWidget(hueShift = new LabelSlider("Hue shift", -6.0f, 6.0f, 0.0f));
		sliders->addWidget(temperature = new TemperatureSlider("White balance"));
		sliders->addWidget(unsharpMask = new LabelSlider("Unsharp mask", -1.0f, 1.0f, 0.0f));

		connect(contrast, &LabelSlider::valueChanged, this, &ColorCorrect::emitChanged);
		connect(pivot, &LabelSlider::valueChanged, this, &ColorCorrect::emitChanged);
		connect(saturation, &LabelSlider::valueChanged, this, &ColorCorrect::emitChanged);
		connect(hueShift, &LabelSlider::valueChanged, this, &ColorCorrect::emitChanged);
		connect(temperature, &TemperatureSlider::valueChanged, this, &ColorCorrect::emitChanged);
		connect(unsharpMask, &LabelSlider::valueChanged, this, &ColorCorrect::emitChanged);
	}

	GradingSettings state()
	{
		return  {
			lift->value(),
			gamma->value(),
			gain->value(),
			offset->value(),
			contrast->value(),
			pivot->value(),
			saturation->value(),
			hueShift->value(),
			temperature->value(),
			unsharpMask->value()
		};
	}

signals:
	void changed(GradingSettings);
};

class CCPreview : public QOpenGLWidget
{
	Q_OBJECT;

	GradingSettings state;
	ColorBufferObject2D screens[4] = { 
		ColorBufferObject2D::fromQImage(QGLWidget::convertToGLFormat(QImage("../screens/01.png")), false, true),
		ColorBufferObject2D::fromQImage(QGLWidget::convertToGLFormat(QImage("../screens/02.png")), false, true) ,
		ColorBufferObject2D::fromQImage(QGLWidget::convertToGLFormat(QImage("../screens/03.png")), false, true) ,
		ColorBufferObject2D::fromQImage(QGLWidget::convertToGLFormat(QImage("../screens/04.png")), false, true) };
	Program program;
	int imageIndex = 0;

public:
	void set(GradingSettings state)
	{
		// receive settings
		this->state = state;
		// kick off a repaint
		repaint();
	}

	virtual void initializeGL() override
	{
		gl.initializeOpenGLFunctions();

		// load a shader to see grading in action
		Shader shader("../grading.glsl", ProgramStage::frag);
		program = Program(shader);

		setFocusPolicy(Qt::StrongFocus);
	}

	virtual void keyPressEvent(QKeyEvent* event) override
	{
		if (event->key() == Qt::Key_Space)
		{
			imageIndex = (imageIndex + 1) % 4;
			repaint();
		}
	}

	virtual void resizeGL(int w, int h) override
	{
		glViewport(0, 0, w, h);
	}

	virtual void paintGL() override
	{
		program.bind();
		program.set("uResolution", (float)width(), (float)height());
		program.set("uImages[1]", 0, screens[imageIndex]);

		program.set("uLift", state.lift);
		program.set("uGamma", state.gamma);
		program.set("uGain", state.gain);
		program.set("uOffset", state.offset);
		program.set("uContrast", state.contrast);
		program.set("uContrastPivot", state.pivot);
		program.set("uSaturation", state.saturation);
		program.set("uHue", state.hueShift);
		program.set("uTemperature", state.temperature);
		program.set("uUnsharpMask", state.unsharpMask);

		glRecti(-1, -1, 1, 1);
	}
};

class ColorGradingApp : public QMainWindow
{
	Q_OBJECT;

	QSplitter* main;

public:
	ColorGradingApp()
	{
		ColorCorrect* cc;
		CCPreview* view;
		setCentralWidget(main = new QSplitter(Qt::Vertical));
		main->addWidget(view = new CCPreview());
		main->addWidget(cc = new ColorCorrect());
		connect(cc, &ColorCorrect::changed, view, &CCPreview::set);
		view->set(cc->state());
	}

#pragma warning(suppress: 4100)
	virtual void showEvent(QShowEvent* event) override
	{
		QSettings settings("cg.ini", QSettings::IniFormat);
		QVariant state = settings.value("state");
		if(state.type() == QVariant::ByteArray)
			restoreState(state.toByteArray());
		QVariant geo = settings.value("geo");
		if (geo.type() == QVariant::ByteArray)
			restoreGeometry(geo.toByteArray());
		QVariant splitterstate = settings.value("splitterstate");
		if (splitterstate.type() == QVariant::ByteArray)
			main->restoreState(splitterstate.toByteArray());
	}

#pragma warning(suppress: 4100)
	virtual void closeEvent(QCloseEvent* event) override
	{
		QSettings settings("cg.ini", QSettings::IniFormat);
		settings.setValue("sate", saveState());
		settings.setValue("geo", saveGeometry());
		settings.setValue("splitterstate", main->saveState());
	}
};

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	ColorGradingApp w;
	w.show();
	return a.exec();
}

// this is how you do Q_OBJECT macros inside cpp files
#ifndef Q_MOC_RUN
#ifdef _DEBUG
#include "GeneratedFiles/debug/main.moc"
#else
#include "GeneratedFiles/release/main.moc"
#endif
#endif

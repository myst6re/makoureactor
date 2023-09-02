#include "QColorPicker.h"
#include <PsColor>

static int pWidth = 220;
static int pHeight = 200;

QColorShowLabel::QColorShowLabel(QWidget *parent)
    : QFrame(parent)
{
	setFrameStyle(QFrame::Panel | QFrame::Sunken);
}

void QColorShowLabel::setColor(QColor c)
{
	col = c;
	repaint();
}

void QColorShowLabel::paintEvent(QPaintEvent *e)
{
	QPainter p(this);
	drawFrame(&p);
	p.fillRect(contentsRect() & e->rect(), col);
}

void QColorShowLabel::mousePressEvent(QMouseEvent *m)
{
	Q_UNUSED(m)

	emit clicked();
}

int QColorLuminancePicker::y2val(int y)
{
	int d = height() - 2 * coff - 1;
	
	return 255 - (y - coff) * 255 / d;
}

int QColorLuminancePicker::val2y(int v)
{
	int d = height() - 2 * coff - 1;
	
	return coff + (255 - v) * d / 255;
}

QColorLuminancePicker::QColorLuminancePicker(QWidget* parent)
    :QWidget(parent)
{
	hue = 100; val = 100; sat = 100;
	pix = 0;
}

QColorLuminancePicker::~QColorLuminancePicker()
{
	delete pix;
}

void QColorLuminancePicker::mouseMoveEvent(QMouseEvent *m)
{
	setVal(y2val(m->y()));
}

void QColorLuminancePicker::mousePressEvent(QMouseEvent *m)
{
	setVal(y2val(m->y()));
}

void QColorLuminancePicker::setVal(int v)
{
	if (val == v) {
		return;
	}
	
	val = std::max(0, std::min(v, 255));
	delete pix;
	pix = nullptr;
	repaint();
	
	emit newHsv(hue, sat, val);
}

// Receives from a hue,sat chooser and relays.
void QColorLuminancePicker::setCol(int h, int s)
{
	setCol(h, s, val);
	
	emit newHsv(h, s, val);
}

void QColorLuminancePicker::paintEvent(QPaintEvent *)
{
	int w = width() - 5;
	QRect r(0, foff, w, height() - 2 * foff);
	int wi = r.width() - 2;
	int hi = r.height() - 2;
	
	if (!pix || pix->height() != hi || pix->width() != wi) {
		delete pix;
		QImage img(wi, hi, QImage::Format_RGB32);
		int y;
		uint *pixel = (uint *)img.scanLine(0);
		
		for (y = 0; y < hi; y++) {
			const uint *end = pixel + wi;
			while (pixel < end) {
				QColor c;
				c.setHsv(hue, sat, y2val(y + coff));
				*pixel = PsColor::fromPsColor(PsColor::toPsColor(c.rgb()));
				++pixel;
			}
		}
		
		pix = new QPixmap(QPixmap::fromImage(img));
	}
	
	QPainter p(this);
	if (isEnabled()) {
		p.drawPixmap(1, coff, *pix);
		const QPalette &g = palette();
		qDrawShadePanel(&p, r, g, true);
		p.setPen(g.windowText().color());
		p.setBrush(g.windowText());
		QPolygon a;
		int y = val2y(val);
		a.setPoints(3, w, y, w + 5, y + 5, w + 5, y - 5);
		p.eraseRect(w, 0, 5, height());
		p.drawPolygon(a);
	} else {
		p.drawImage(1, coff, pix->toImage().convertToFormat(QImage::Format_Grayscale8));
	}
}
void QColorLuminancePicker::setCol(int h, int s , int v)
{
	val = v;
	hue = h;
	sat = s;
	delete pix;
	pix = nullptr;
	repaint();
}

QPoint QColorPicker::colPt()
{
	QRect r = contentsRect();
	
	return QPoint((360 - hue) * (r.width() - 1) / 360, (255 - sat) * (r.height() - 1) / 255);
}

int QColorPicker::huePt(const QPoint &pt)
{
	QRect r = contentsRect();
	
	return 360 - pt.x() * 360 / (r.width() - 1);
}

int QColorPicker::satPt(const QPoint &pt)
{
	QRect r = contentsRect();
	
	return 255 - pt.y() * 255 / (r.height() - 1);
}

void QColorPicker::setCol(const QPoint &pt)
{
	setCol(huePt(pt), satPt(pt));
}

QColorPicker::QColorPicker(QWidget* parent)
    : QFrame(parent)
{
	hue = 0;
	sat = 0;
	setCol(150, 255);
	setAttribute(Qt::WA_NoSystemBackground);
	setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
}

QColorPicker::~QColorPicker()
{
}

QSize QColorPicker::sizeHint() const
{
	return QSize(pWidth + 2 * frameWidth(), pHeight + 2 * frameWidth());
}

void QColorPicker::setCol(int h, int s)
{
	int nhue = std::min(std::max(0, h), 359);
	int nsat = std::min(std::max(0, s), 255);
	
	if (nhue == hue && nsat == sat) {
		return;
	}
	
	QRect r(colPt(), QSize(20, 20));
	hue = nhue;
	sat = nsat;
	r = r.united(QRect(colPt(), QSize(20, 20)));
	r.translate(contentsRect().x() - 9, contentsRect().y() - 9);
	repaint(r);
}

void QColorPicker::mouseMoveEvent(QMouseEvent *m)
{
	QPoint p = m->pos() - contentsRect().topLeft();
	setCol(p);
	
	emit newCol(hue, sat);
}

void QColorPicker::mousePressEvent(QMouseEvent *m)
{
	QPoint p = m->pos() - contentsRect().topLeft();
	setCol(p);
	
	emit newCol(hue, sat);
}

void QColorPicker::paintEvent(QPaintEvent *event)
{
	Q_UNUSED(event)
	
	QPainter p(this);
	drawFrame(&p);
	QRect r = contentsRect();
	if (isEnabled()) {
		p.drawPixmap(r.topLeft(), pix);
		QPoint pt = colPt() + r.topLeft();
		p.setPen(Qt::black);
		p.fillRect(pt.x() - 9, pt.y(), 20, 2, Qt::black);
		p.fillRect(pt.x(), pt.y() - 9, 2, 20, Qt::black);
	} else {
		p.drawImage(r.topLeft(), pix.toImage().convertToFormat(QImage::Format_Grayscale8));
	}
}

void QColorPicker::resizeEvent(QResizeEvent *ev)
{
	QFrame::resizeEvent(ev);
	
	int w = width() - frameWidth() * 2;
	int h = height() - frameWidth() * 2;
	QImage img(w, h, QImage::Format_RGB32);
	int x, y;
	uint *pixel = (uint *) img.scanLine(0);
	
	for (y = 0; y < h; y++) {
		const uint *end = pixel + w;
		x = 0;
		while (pixel < end) {
			QPoint p(x, y);
			QColor c;
			c.setHsv(huePt(p), satPt(p), 200);
			*pixel = PsColor::fromPsColor(PsColor::toPsColor(c.rgb()));
			++pixel;
			++x;
		}
	}
	
	pix = QPixmap::fromImage(img);
}

#pragma once

#include <QtWidgets>

class QColorShowLabel : public QFrame
{
	Q_OBJECT
public:
	QColorShowLabel(QWidget *parent = nullptr);
	inline const QColor &color() const {
		return col;
	}
	void setColor(QColor c);
signals:
	void clicked();
protected:
	void paintEvent(QPaintEvent *event) override;
	void mousePressEvent(QMouseEvent *event) override;
private:
	QColor col;
};

class QColorLuminancePicker : public QWidget
{
	Q_OBJECT
public:
	QColorLuminancePicker(QWidget *parent = nullptr);
	~QColorLuminancePicker();
public slots:
	void setCol(int h, int s, int v);
	void setCol(int h, int s);
signals:
	void newHsv(int h, int s, int v);
protected:
	void paintEvent(QPaintEvent *event);
	void mouseMoveEvent(QMouseEvent *event);
	void mousePressEvent(QMouseEvent *event);
private:
	enum { foff = 3, coff = 4 }; // frame and contents offset
	int val;
	int hue;
	int sat;
	int y2val(int y);
	int val2y(int val);
	void setVal(int v);
	QPixmap *pix;
};

class QColorPicker : public QFrame
{
	Q_OBJECT
public:
	QColorPicker(QWidget *parent = nullptr);
	~QColorPicker();
public slots:
	void setCol(int h, int s);
signals:
	void newCol(int h, int s);
protected:
	QSize sizeHint() const;
	void paintEvent(QPaintEvent *event);
	void mouseMoveEvent(QMouseEvent *event);
	void mousePressEvent(QMouseEvent *event);
	void resizeEvent(QResizeEvent *event);
private:
	int hue;
	int sat;
	QPoint colPt();
	int huePt(const QPoint &pt);
	int satPt(const QPoint &pt);
	void setCol(const QPoint &pt);
	QPixmap pix;
	QList<QColor> _colors;
};

/****************************************************************************
 ** Deling Final Fantasy VIII Field Editor
 ** Copyright (C) 2009-2012 Arzel Jérôme <myst6re@gmail.com>
 **
 ** This program is free software: you can redistribute it and/or modify
 ** it under the terms of the GNU General Public License as published by
 ** the Free Software Foundation, either version 3 of the License, or
 ** (at your option) any later version.
 **
 ** This program is distributed in the hope that it will be useful,
 ** but WITHOUT ANY WARRANTY; without even the implied warranty of
 ** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 ** GNU General Public License for more details.
 **
 ** You should have received a copy of the GNU General Public License
 ** along with this program.  If not, see <http://www.gnu.org/licenses/>.
 ****************************************************************************/
#ifndef ORIENTATIONWIDGET_H
#define ORIENTATIONWIDGET_H

#include <QtGui>

class OrientationWidget : public QWidget
{
	Q_OBJECT
public:
	explicit OrientationWidget(QWidget *parent = 0);
	explicit OrientationWidget(quint8 value, QWidget *parent = 0);
	quint8 value() const;
	bool isReadOnly() const;
	void setReadOnly(bool ro);
	virtual QSize minimumSizeHint() const;
	virtual QSize sizeHint() const;
signals:
	void valueChanged(int i);
	void valueEdited(int i);
public slots:
	void setValue(int value);
private:
	void byte2degree(quint8 v);
	quint8 degree2byte() const;
	QPointF centerCircle() const;
	double radiusCircle() const;
	bool isInCircle(const QPointF &pos);
	void moveCursor(const QPointF &pos);
	int _value;
	bool _readOnly;
protected:
	virtual void paintEvent(QPaintEvent *e);
	virtual void mousePressEvent(QMouseEvent *e);
	virtual void mouseMoveEvent(QMouseEvent *e);
};

#endif // ORIENTATIONWIDGET_H

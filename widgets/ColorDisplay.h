/****************************************************************************
 ** Makou Reactor Final Fantasy VII Field Script Editor
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
#ifndef COLORDISPLAY_H
#define COLORDISPLAY_H

#include <QtGui>

#define COLOR_DISPLAY_CELL_SIZE		9
#define COLOR_DISPLAY_BORDER_WIDTH	1

class ColorDisplay : public QWidget
{
	Q_OBJECT
public:
	explicit ColorDisplay(QWidget *parent=0);
	void setColors(const QList<QRgb> &colors);
	const QList<QRgb> &getColors() const;
	bool isReadOnly() const;
	void setReadOnly(bool ro);
signals:
	void colorEdited(int id, QRgb value);
	void colorHovered(int id);
private:
	int colorId(const QPoint &pos) const;
	QList<QRgb> colors;
	bool _ro;
protected:
	void paintEvent(QPaintEvent *event);
	void enterEvent(QMouseEvent *event);
	void leaveEvent(QMouseEvent *event);
	void mouseMoveEvent(QMouseEvent *event);
	void mouseReleaseEvent(QMouseEvent *event);
};

#endif // COLORDISPLAY_H

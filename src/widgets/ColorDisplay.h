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

#include <QtWidgets>

#define COLOR_DISPLAY_MIN_CELL_SIZE   9
#define COLOR_DISPLAY_BORDER_WIDTH    1

class ColorDisplay : public QWidget
{
	Q_OBJECT
public:
	explicit ColorDisplay(QWidget *parent = nullptr);
	void setColors(const QList<QRgb> &colors);
	const QList<QRgb> &colors() const;
	bool isReadOnly() const;
	void setReadOnly(bool ro);
	QSize sizeHint() const;
	QSize minimumSizeHint() const;
	inline int cellWidth() const {
		return cellWidth(width());
	}
	inline int cellHeight() const {
		return cellHeight(height());
	}
signals:
	void colorEdited(int id, QRgb value);
	void colorHovered(int id);
private:
	int colorId(const QPoint &pos) const;
	int cellWidth(int w) const;
	int cellHeight(int h) const;
	QSize cellSize() const;
	QList<QRgb> _colors;
	bool _ro, _hover;
protected:
	void paintEvent(QPaintEvent *event);
	void enterEvent(QEvent *event);
	void leaveEvent(QEvent *event);
	void mouseMoveEvent(QMouseEvent *event);
	void mouseReleaseEvent(QMouseEvent *event);
};

#endif // COLORDISPLAY_H

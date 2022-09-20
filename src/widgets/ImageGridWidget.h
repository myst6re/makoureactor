/****************************************************************************
 ** Makou Reactor Final Fantasy VII Field Script Editor
 ** Copyright (C) 2009-2022 Arzel Jérôme <myst6re@gmail.com>
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
#pragma once

#include <QtWidgets>

class ImageGridWidget : public QWidget
{
	Q_OBJECT
public:
	explicit ImageGridWidget(QWidget *parent = nullptr);
	inline const QPixmap &pixmap() const {
		return _pixmap;
	}
	void setPixmap(const QPixmap &pixmap);
	void setCellSize(int size);
	QPixmap cellPixmap(const QPoint &point) const;
signals:
	void currentCellChanged(const QPoint &point);
	void highlighted(const QPoint &point);
public slots:
	void setCurrentCell(const QPoint &point);
protected:
	virtual void paintEvent(QPaintEvent *event) override;
	virtual void mouseMoveEvent(QMouseEvent *event) override;
	virtual void leaveEvent(QEvent *event) override;
	virtual void mouseReleaseEvent(QMouseEvent *event) override;
	virtual void resizeEvent(QResizeEvent *event) override;
private:
	void drawSelection(QPainter &p, QPoint selection);
	QPoint scaledPoint(const QPoint &point) const;
	QPoint getCell(const QPoint &pos) const;
	void updateGrid();
	void updateScaledPixmapSize();

	QPixmap _pixmap;
	QList<QLine> _gridLines;
	QPoint _currentCell, _hoverCell;
	QPoint _scaledPixmapPoint;
	QSize _scaledPixmapSize;
	double _scaledRatio;
	int _cellSize;
};


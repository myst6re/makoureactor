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
#include "ImageGridWidget.h"

ImageGridWidget::ImageGridWidget(QWidget *parent)
    : QWidget(parent), _scaledRatio(0.0), _cellSize(0)
{
	setMouseTracking(true);
}

void ImageGridWidget::setPixmap(const QPixmap &pixmap)
{
	_pixmap = pixmap;
	updateScaledPixmapSize();
	update();
}

void ImageGridWidget::setCellSize(int size)
{
	_cellSize = size;
	updateGrid();
	update();
}

QPixmap ImageGridWidget::cellPixmap(const QPoint &point) const
{
	return _pixmap.copy(QRect(point * _cellSize, QSize(_cellSize, _cellSize)));
}

void ImageGridWidget::setCurrentCell(const QPoint &point)
{
	if (_currentCell != point) {
		_currentCell = point;
		update();

		emit currentCellChanged(point);
	}
}

void ImageGridWidget::updateScaledPixmapSize()
{
	_scaledPixmapSize = _pixmap.size().scaled(size(), Qt::KeepAspectRatio);

	_scaledRatio = _pixmap.width() == 0 ? 0.0 : _scaledPixmapSize.width() / double(_pixmap.width());

	_scaledPixmapPoint = QPoint((width() - _scaledPixmapSize.width()) / 2, (height() - _scaledPixmapSize.height()) / 2);
}

QPoint ImageGridWidget::scaledPoint(const QPoint &point) const
{
	return _scaledRatio * point;
}

QPoint ImageGridWidget::getCell(const QPoint &pos) const
{
	if (_cellSize == 0 || _scaledRatio == 0) {
		return QPoint();
	}
	QPointF res = QPointF(pos - _scaledPixmapPoint) / (_scaledRatio * _cellSize);
	return QPoint(qFloor(res.x()), qFloor(res.y()));
}

void ImageGridWidget::updateGrid()
{
	_gridLines.clear();

	if (_cellSize == 0) {
		return;
	}

	const QSize imageSize = _pixmap.size();
	const int lineCountV = imageSize.width() / _cellSize + 1,
	        lineCountH = imageSize.height() / _cellSize + 1;

	for (int i = 0; i < lineCountV; ++i) {
		_gridLines.append(QLine(scaledPoint(QPoint(i * _cellSize, 0)), scaledPoint(QPoint(i * _cellSize, imageSize.height()))));
	}

	for (int i = 0; i < lineCountH; ++i) {
		_gridLines.append(QLine(scaledPoint(QPoint(0, i * _cellSize)), scaledPoint(QPoint(imageSize.width(), i * _cellSize))));
	}
}

void ImageGridWidget::paintEvent(QPaintEvent *event)
{
	Q_UNUSED(event)

	QPainter p(this);

	if (isEnabled()) {
		p.setBrush(Qt::black);
		p.drawRect(0, 0, width(), height());
	}

	p.setPen(Qt::gray);

	p.translate(_scaledPixmapPoint);
	p.drawPixmap(QRect(QPoint(0, 0), _scaledPixmapSize), _pixmap);
	p.drawLines(_gridLines);

	if (isEnabled()) {
		QColor lightRed(0xff, 0x7f, 0x7f);

		p.setPen(hasFocus() ? Qt::red : lightRed);
		drawSelection(p, _currentCell);

		p.setPen(lightRed);
		drawSelection(p, _hoverCell);
	}
}

void ImageGridWidget::drawSelection(QPainter &p, QPoint selection)
{
	if (selection.isNull()) {
		return;
	}

	selection *= _cellSize;
	p.drawLine(QLine(scaledPoint(selection), scaledPoint(selection + QPoint(0, _cellSize))));
	p.drawLine(QLine(scaledPoint(selection), scaledPoint(selection + QPoint(_cellSize, 0))));
	p.drawLine(QLine(scaledPoint(selection + QPoint(_cellSize, 0)), scaledPoint(selection + QPoint(_cellSize, _cellSize))));
	p.drawLine(QLine(scaledPoint(selection + QPoint(0, _cellSize)), scaledPoint(selection + QPoint(_cellSize, _cellSize))));
}

void ImageGridWidget::mouseMoveEvent(QMouseEvent *event)
{
	QPoint newCell = getCell(event->pos());

	if (newCell != _hoverCell) {
		_hoverCell = newCell;
		update();

		emit highlighted(_hoverCell);
	}
}

void ImageGridWidget::leaveEvent(QEvent *event)
{
	Q_UNUSED(event)
	if (!_hoverCell.isNull()) {
		_hoverCell = QPoint();
		update();
	}
}

void ImageGridWidget::mouseReleaseEvent(QMouseEvent *event)
{
	setCurrentCell(getCell(event->pos()));
	setFocus();
}

void ImageGridWidget::resizeEvent(QResizeEvent *event)
{
	updateScaledPixmapSize();
	updateGrid();
	QWidget::resizeEvent(event);
}

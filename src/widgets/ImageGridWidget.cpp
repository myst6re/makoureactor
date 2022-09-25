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
    : QWidget(parent), _hoverCell(-1, -1), _scaledRatio(0.0),
      _selectionMode(SingleSelection), _cellSize(0), _startMousePress(false)
{
	setMouseTracking(_selectionMode != NoSelection);
}

void ImageGridWidget::setPixmap(const QPixmap &pixmap)
{
	_pixmap = pixmap;
	updateScaledPixmapSize();
	updateGrid();
	update();
}

void ImageGridWidget::setCellSize(int size)
{
	_cellSize = size;
	updateGrid();
	update();
}

QPixmap ImageGridWidget::cellPixmap(const Cell &point) const
{
	if (point == Cell(-1, -1)) {
		return QPixmap();
	}
	return _pixmap.copy(QRect(point * _cellSize, QSize(_cellSize, _cellSize)));
}

void ImageGridWidget::setSelectionMode(SelectionMode selectionMode)
{
	_selectionMode = selectionMode;
	setMouseTracking(_selectionMode != NoSelection);
	clearHover();
}

void ImageGridWidget::setSelectedCells(const QList<Cell> &cells)
{
	if (_selectedCells != cells) {
		_selectedCells = cells;
		update();

		emit currentSelectionChanged(cells);
	}
}

void ImageGridWidget::updateScaledPixmapSize()
{
	_scaledPixmapSize = _pixmap.size().scaled(size(), Qt::KeepAspectRatio);

	_scaledRatio = _pixmap.width() == 0 ? 0.0 : _scaledPixmapSize.width() / double(_pixmap.width());

	_scaledPixmapPoint = QPoint((width() - _scaledPixmapSize.width()) / 2, (height() - _scaledPixmapSize.height()) / 2);
}

void ImageGridWidget::clearHover()
{
	Cell newCell(-1, -1);

	if (newCell != _hoverCell) {
		_hoverCell = newCell;
		update();
	}
}

QPoint ImageGridWidget::scaledPoint(const Cell &point) const
{
	return _scaledRatio * point;
}

Cell ImageGridWidget::getCell(const QPoint &pos) const
{
	if (_cellSize == 0 || _scaledRatio == 0.0) {
		return Cell(-1, -1);
	}
	QPointF cell = QPointF(pos - _scaledPixmapPoint) / (_scaledRatio * _cellSize);
	Cell ret(qFloor(cell.x()), qFloor(cell.y()));
	return cellIsInRange(ret) ? ret : Cell(-1, -1);
}

bool ImageGridWidget::cellIsInRange(const Cell &point) const
{
	return _pixmap.rect().contains(point * _cellSize);
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
	QPainter p(this);

	if (isEnabled()) {
		p.setBrush(Qt::black);
		p.drawRect(0, 0, width(), height());
	}

	p.translate(_scaledPixmapPoint);

	if (isEnabled()) {
		p.drawPixmap(QRect(QPoint(), _scaledPixmapSize), _pixmap);
		p.setPen(Qt::gray);
		p.drawLines(_gridLines);

		QColor lightRed(0xff, 0x7f, 0x7f);

		p.setPen(hasFocus() ? Qt::red : lightRed);
		for (const Cell &cell: _selectedCells) {
			drawSelection(p, cell);
		}

		p.setPen(lightRed);
		drawSelection(p, _hoverCell);
	} else {
		QStyleOption opt;
		opt.initFrom(this);
		p.drawPixmap(QRect(QPoint(), _scaledPixmapSize), QWidget::style()->generatedIconPixmap(QIcon::Disabled, _pixmap, &opt));
	}

	QWidget::paintEvent(event);
}

void ImageGridWidget::drawSelection(QPainter &p, QPoint selection)
{
	if (selection == Cell(-1, -1)) {
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
	QWidget::mouseMoveEvent(event);

	if (_selectionMode == NoSelection) {
		return;
	}

	Cell newCell = getCell(event->pos());

	if (_startMousePress) {
		_selectedCells.append(newCell);
		// Unique values
		setSelectedCells(QSet<Cell>(_selectedCells.begin(), _selectedCells.end()).values());
		return;
	}

	if (newCell != _hoverCell) {
		_hoverCell = newCell;
		update();

		emit highlighted(_hoverCell);
	}
}

void ImageGridWidget::leaveEvent(QEvent *event)
{
	QWidget::leaveEvent(event);

	if (_selectionMode == NoSelection) {
		return;
	}

	clearHover();
}

void ImageGridWidget::mousePressEvent(QMouseEvent *event)
{
	if (_selectionMode != NoSelection) {
		setSelectedCell(getCell(event->pos()));
		setFocus();
		_startMousePress = _selectionMode == MultiSelection;
	}

	QWidget::mousePressEvent(event);
}

void ImageGridWidget::mouseReleaseEvent(QMouseEvent *event)
{
	_startMousePress = false;

	QWidget::mouseReleaseEvent(event);
}

void ImageGridWidget::keyPressEvent(QKeyEvent *event)
{
	QWidget::keyPressEvent(event);

	if (_selectedCells.isEmpty()) {
		return;
	}

	Cell cell = _selectedCells.last();

	switch (event->key()) {
	case Qt::Key_Left:
		cell.setX(cell.x() - 1);
		break;
	case Qt::Key_Right:
		cell.setX(cell.x() + 1);
		break;
	case Qt::Key_Up:
		cell.setY(cell.y() - 1);
		break;
	case Qt::Key_Down:
		cell.setY(cell.y() + 1);
		break;
	}

	if (cellIsInRange(cell)) {
		setSelectedCell(cell);
	}
}

void ImageGridWidget::resizeEvent(QResizeEvent *event)
{
	updateScaledPixmapSize();
	updateGrid();
	QWidget::resizeEvent(event);
}

QSize ImageGridWidget::minimumSizeHint() const
{
	return _pixmap.isNull() ? QWidget::minimumSizeHint() : _pixmap.size() / _cellSize * 4;
}

QSize ImageGridWidget::sizeHint() const
{
	return _pixmap.isNull() ? QWidget::sizeHint() : _pixmap.size();
}

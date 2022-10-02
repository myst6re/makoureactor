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
    : QWidget(parent), _hoverCell(-1, -1), _pixmapPoint(-1, -1), _scaledRatio(0.0),
      _selectionMode(SingleSelection), _cellSize(0), _startMousePress(false)
{
	setMouseTracking(_selectionMode != NoSelection);
}

void ImageGridWidget::setPixmap(const QPixmap &pixmap)
{
	_pixmap = pixmap;
	updateGrid();
	update();
}

void ImageGridWidget::setPixmap(const QPoint &point, const QPixmap &pixmap)
{
	_pixmapPoint = point;
	_pixmap = pixmap;
	updateGrid();
	update();
}

void ImageGridWidget::setPixmapPoint(const QPoint &point)
{
	_pixmapPoint = point;
	updateGrid();
	update();
}

void ImageGridWidget::setCellSize(int size)
{
	_cellSize = size;
	updateGrid();
	update();
}

void ImageGridWidget::setGridSize(const QSize &gridSize)
{
	_gridSize = gridSize;
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

void ImageGridWidget::clearHover()
{
	Cell newCell(-1, -1);

	if (newCell != _hoverCell) {
		_hoverCell = newCell;
		update();
	}
}

Cell ImageGridWidget::getCell(const QPoint &pos) const
{
	if (_cellSize == 0 || _scaledRatio == 0.0) {
		return Cell(-1, -1);
	}
	QPointF cell = QPointF(pos - _scaledGridPoint) / (_scaledRatio * _cellSize);
	Cell ret(qFloor(cell.x()), qFloor(cell.y()));
	return cellIsInRange(ret) ? ret : Cell(-1, -1);
}

bool ImageGridWidget::cellIsInRange(const Cell &point) const
{
	return QRect(QPoint(0, 0), _gridSize).contains(point);
}

void ImageGridWidget::updateGrid()
{
	_gridLines.clear();

	if (_cellSize == 0) {
		return;
	}

	const QSize gridS = gridSize();
	const int lineCountV = gridS.width() + 1,
	        lineCountH = gridS.height() + 1;

	_scaledRatio = gridS.width() == 0 ? 0.0 : (gridS * _cellSize).scaled(size(), Qt::KeepAspectRatio).width() / double(gridS.width() * _cellSize);

	qDebug() << "updateGrid" << _scaledRatio << lineCountV << lineCountH << gridS << _pixmap.size() << _gridSize << size();

	for (int i = 0; i < lineCountV; ++i) {
		_gridLines.append(QLine(scaledPoint(QPoint(i * _cellSize, 0)), scaledPoint(QPoint(i, gridS.height()) * _cellSize)));
	}

	for (int i = 0; i < lineCountH; ++i) {
		_gridLines.append(QLine(scaledPoint(QPoint(0, i * _cellSize)), scaledPoint(QPoint(gridS.width(), i) * _cellSize)));
	}

	_scaledGridPoint = (QPoint(width(), height()) - scaledPoint(QPoint(gridS.width(), gridS.height()) * _cellSize)) / 2;
	_scaledPixmapPoint = _pixmapPoint == QPoint(-1, -1) ? _scaledGridPoint : scaledPoint(_pixmapPoint);
}

void ImageGridWidget::paintEvent(QPaintEvent *event)
{
	QPainter p(this);

	if (isEnabled()) {
		// Background
		p.setBrush(Qt::black);
		p.drawRect(0, 0, width(), height());
		// Grid Background
		p.setBrush(palette().color(QPalette::Dark));
		p.drawRect(QRect(_scaledGridPoint, gridSizePixel() * _scaledRatio));
		// Pixmap
		p.drawPixmap(QRect(_scaledPixmapPoint, _pixmap.size() * _scaledRatio), _pixmap);
		// Grid
		p.setPen(Qt::gray);
		p.translate(_scaledGridPoint);
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
		p.drawPixmap(QRect(_scaledPixmapPoint, _pixmap.size() * _scaledRatio), QWidget::style()->generatedIconPixmap(QIcon::Disabled, _pixmap, &opt));
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
	updateGrid();
	QWidget::resizeEvent(event);
}

QSize ImageGridWidget::minimumSizeHint() const
{
	return gridSize() * 8;
}

QSize ImageGridWidget::sizeHint() const
{
	return minimumSizeHint();
}

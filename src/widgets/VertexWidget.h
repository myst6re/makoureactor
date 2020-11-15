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
#ifndef VERTEXWIDGET_H
#define VERTEXWIDGET_H

#include <QtWidgets>
#include "core/field/IdFile.h"

class VertexWidget : public QWidget
{
	Q_OBJECT
public:
	explicit VertexWidget(QWidget *parent);
	VertexWidget(const QString &xLabel=QString(),
				 const QString &yLabel=QString(),
				 const QString &zLabel=QString(),
	             QWidget *parent = nullptr);
	Vertex_s values() const;
	void setValues(const Vertex_s &v);
	bool isReadOnly() const;
	void setReadOnly(bool ro);
private slots:
	void emitValuesChanged();
signals:
	void valuesChanged(const Vertex_s &v);
private:
	void build(const QString &xLabel = QString(),
	           const QString &yLabel = QString(),
	           const QString &zLabel = QString());
	QSpinBox *x, *y, *z;
	bool dontEmit;
};

#endif // VERTEXWIDGET_H

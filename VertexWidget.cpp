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
#include "VertexWidget.h"

VertexWidget::VertexWidget(QWidget *parent) :
	QWidget(parent), dontEmit(false)
{
	x = new QSpinBox;
	x->setRange(-32768, 32767);
	y = new QSpinBox;
	y->setRange(-32768, 32767);
	z = new QSpinBox;
	z->setRange(-32768, 32767);

	QHBoxLayout *layout = new QHBoxLayout(this);
	layout->setContentsMargins(QMargins());
	layout->addWidget(new QLabel(tr("X")));
	layout->addWidget(x, 1);
	layout->addWidget(new QLabel(tr("Y")));
	layout->addWidget(y, 1);
	layout->addWidget(new QLabel(tr("Z")));
	layout->addWidget(z, 1);

	connect(x, SIGNAL(valueChanged(int)), SLOT(emitValuesChanged()));
	connect(y, SIGNAL(valueChanged(int)), SLOT(emitValuesChanged()));
	connect(z, SIGNAL(valueChanged(int)), SLOT(emitValuesChanged()));
}

Vertex_s VertexWidget::values() const
{
	Vertex_s v;
	v.x = x->value();
	v.y = y->value();
	v.z = z->value();
	return v;
}

void VertexWidget::setValues(const Vertex_s &v)
{
	x->setValue(v.x);
	y->setValue(v.y);
	z->setValue(v.z);
}

void VertexWidget::emitValuesChanged()
{
	if(!dontEmit) {
		emit valuesChanged(values());
	}
}

bool VertexWidget::isReadOnly() const
{
	return x->isReadOnly();
}

void VertexWidget::setReadOnly(bool ro)
{
	x->setReadOnly(ro);
	y->setReadOnly(ro);
	z->setReadOnly(ro);
}

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
#include "ModelColorsLayout.h"

ModelColorWidget::ModelColorWidget(QWidget *parent) :
      QObject(parent)
{
	_colorWidget = new ColorDisplay(parent);
	_colorWidget->setColors(QList<QRgb>() << Qt::black);

	for (quint8 i = 0; i < 3; ++i) {
		_dirWidget[i] = new QSpinBox(parent);
		_dirWidget[i]->setRange(-32768, 32767);
		connect(_dirWidget[i], &QSpinBox::editingFinished, this, &ModelColorWidget::relayEdition);
	}

	_dirWidget[0]->adjustSize();
	_colorWidget->setMinimumWidth(_dirWidget[0]->height());
	_colorWidget->setMinimumHeight(_dirWidget[0]->height());

	connect(_colorWidget, &ColorDisplay::colorEdited, this, &ModelColorWidget::relayEdition);
}

void ModelColorWidget::relayEdition()
{
	emit colorDirEdited(FieldModelColorDir(
	                        qint16(_dirWidget[0]->value()),
	                        qint16(_dirWidget[1]->value()),
	                        qint16(_dirWidget[2]->value()),
	                        _colorWidget->colors().first()));
}

void ModelColorWidget::setModelColorDir(const FieldModelColorDir &dir)
{
	_colorWidget->setColors(QList<QRgb>() << dir.color);
	_dirWidget[0]->setValue(dir.dirA);
	_dirWidget[1]->setValue(dir.dirB);
	_dirWidget[2]->setValue(dir.dirC);
}

void ModelColorWidget::setReadOnly(bool ro)
{
	_colorWidget->setReadOnly(ro);
	for (quint8 i = 0; i < 3; ++i) {
		_dirWidget[i]->setReadOnly(ro);
	}
}

ModelColorsLayout::ModelColorsLayout(QWidget *parent) :
    QGridLayout(parent)
{
	addWidget(new QLabel(tr("Color")), 0, 0);
	addWidget(new QLabel(tr("X")), 0, 1);
	addWidget(new QLabel(tr("Y")), 0, 2);
	addWidget(new QLabel(tr("Z")), 0, 3);

	for (quint8 i = 0; i < 3; ++i) {
		modelColorWidget[i] = new ModelColorWidget(parent);
		addWidget(modelColorWidget[i]->colorWidget(), i + 1, 0);
		addWidget(modelColorWidget[i]->dirWidget(0), i + 1, 1);
		addWidget(modelColorWidget[i]->dirWidget(1), i + 1, 2);
		addWidget(modelColorWidget[i]->dirWidget(2), i + 1, 3);
	}

	connect(modelColorWidget[0], &ModelColorWidget::colorDirEdited, this, &ModelColorsLayout::relayColorDirEdited0);
	connect(modelColorWidget[1], &ModelColorWidget::colorDirEdited, this, &ModelColorsLayout::relayColorDirEdited1);
	connect(modelColorWidget[2], &ModelColorWidget::colorDirEdited, this, &ModelColorsLayout::relayColorDirEdited2);
}

void ModelColorsLayout::setModelColorDirs(const QList<FieldModelColorDir> &dirs)
{
	if (dirs.size() != 3) {
		qWarning() << "ModelColorsLayout::setModelColorDirs Invalid dirs size";
		return;
	}
	for (quint8 i = 0; i < 3; ++i) {
		modelColorWidget[i]->setModelColorDir(dirs.at(i));
	}
}

void ModelColorsLayout::setReadOnly(bool ro)
{
	for (quint8 i = 0; i < 3; ++i) {
		modelColorWidget[i]->setReadOnly(ro);
	}
}

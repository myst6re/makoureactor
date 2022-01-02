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
#include "ColorDisplay.h"
#include "core/field/FieldModelLoader.h"

class ModelColorWidget : public QObject
{
	Q_OBJECT
public:
	explicit ModelColorWidget(QWidget *parent = nullptr);
	void setModelColorDir(const FieldModelColorDir &dir);
	void setReadOnly(bool ro);
	inline ColorDisplay *colorWidget() const {
		return _colorWidget;
	}
	inline QSpinBox *dirWidget(int id) const {
		return _dirWidget[id];
	}
signals:
	void colorDirEdited(const FieldModelColorDir &colorDir);
private slots:
	void relayEdition();
private:
	ColorDisplay *_colorWidget;
	QSpinBox *_dirWidget[3];
};

class ModelColorsLayout : public QGridLayout
{
	Q_OBJECT
public:
	explicit ModelColorsLayout(QWidget *parent = nullptr);
	void setModelColorDirs(const QList<FieldModelColorDir> &dirs);
	void setReadOnly(bool ro);
signals:
	void colorDirEdited(int id, const FieldModelColorDir &colorDir);
private slots:
	void relayColorDirEdited0(const FieldModelColorDir &colorDir) {
		emit colorDirEdited(0, colorDir);
	}
	void relayColorDirEdited1(const FieldModelColorDir &colorDir) {
		emit colorDirEdited(1, colorDir);
	}
	void relayColorDirEdited2(const FieldModelColorDir &colorDir) {
		emit colorDirEdited(2, colorDir);
	}

private:
	ModelColorWidget *modelColorWidget[3];
};

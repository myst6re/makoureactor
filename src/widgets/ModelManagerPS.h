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
#include "core/field/FieldPS.h"
#include "ColorDisplay.h"
#include "ModelManager.h"

class ModelManagerPS : public ModelManager
{
	Q_OBJECT
public:
	ModelManagerPS(QWidget *parent = nullptr);
protected:
	QList<QStringList> modelNames() const override;
	QList<QTreeWidgetItem *> animItems(int modelID) const override;
	FieldModelFile *modelData(QTreeWidgetItem *item) override;
	FieldModelLoaderPS *modelLoader() const override;
	FieldPS *field() const override;
	const QList<FieldModelColorDir> &lightColors(int modelID) const override;
	void setLightColor(int modelID, int id, const FieldModelColorDir &color) override;
	QRgb globalColor(int modelID) const override;
	void setGlobalColor(int modelID, QRgb color) override;
	quint16 modelScale(int modelID) const override;
	void setModelScale(int modelID, quint16 scale) override;
};

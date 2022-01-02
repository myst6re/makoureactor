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

#include <QtCore>
#include <QRgb>
#include "FieldModelLoader.h"

struct FieldModelInfosPC
{
	QString nameChar, nameHRC;
	QStringList anims;
	QList<quint16> animsUnknown;
	quint16 unknown, typeHRC;
	QList<FieldModelColorDir> colors;
	QRgb globalColor;
};

class FieldModelLoaderPC : public FieldModelLoader
{
public:
	explicit FieldModelLoaderPC(Field *field);
	void clear() override;
	void clean();
	void initEmpty() override;
	bool open() override;
	bool open(const QByteArray &data) override;
	QByteArray save() const override;
	qsizetype modelCount() const override;
	bool insertModel(int modelID, const QString &hrcName);
	void removeModel(int modelID);
	void swapModel(int oldModelID, int newModelID);
	const QStringList &HRCNames() const;
	QString HRCName(int modelID) const;
	void setHRCName(int modelID, const QString &HRCName);
	const QStringList &charNames() const;
	QString charName(int modelID) const;
	void setCharName(int modelID, const QString &charName);
	quint16 scale(int modelID) const;
	void setScale(int modelID, quint16 scale);
	quint16 unknown(int modelID) const override;
	void setUnknown(int modelID, quint16 unknown) override;
	const QList<FieldModelColorDir> &lightColors(int modelID) const;
	void setLightColors(int modelID,
	                    const QList<FieldModelColorDir> &lightColors);
	void setLightColor(int modelID, int colorID,
	                   const FieldModelColorDir &lightColor);
	QRgb globalColor(int modelID) const;
	void setGlobalColor(int modelID, QRgb globalColor);
	qsizetype animCount(int modelID) const override;
	bool insertAnim(int modelID, int numA, const QString &name);
	void removeAnim(int modelID, int numA);
	void swapAnim(int modelID, int oldNumA, int newNumA);
	const QStringList &ANames(int modelID) const;
	QString AName(int modelID, int numA = 0) const;
	void setAName(int modelID, int numA, const QString &animName);
	quint16 animUnknown(int modelID, int numA) const;
	void setAnimUnknown(int modelID, int numA, quint16 unknown);
	FieldModelInfosPC modelInfos(int modelID) const;
	void setModelInfos(int modelID, const FieldModelInfosPC &modelInfos);
	void insertModelInfos(int modelID, const FieldModelInfosPC &modelInfos);
private:
	QStringList model_nameChar;
	QStringList model_nameHRC;
	QList<QStringList> model_anims;
	QList< QList<quint16> > model_anims_unknown;
	QList<quint16> model_unknown;
	QList<quint16> model_typeHRC;
	QList< QList<FieldModelColorDir> > colors;
	QList<QRgb> model_global_color;
};

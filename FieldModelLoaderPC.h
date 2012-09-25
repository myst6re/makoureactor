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
#ifndef FIELDMODELLOADERPC_H
#define FIELDMODELLOADERPC_H

#include <QtCore>
#include <QRgb>
#include "FieldModelLoader.h"

class FieldModelLoaderPC : public FieldModelLoader
{
public:
	FieldModelLoaderPC();
	void clear();
	bool load(const QByteArray &data);
	QByteArray save() const;
	quint16 globalScale() const;
	void setGlobalScale(quint16 scale);
	int modelCount() const;
	void insertModel(int modelID, const QString &hrcName);
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
	quint16 unknown(int modelID) const;
	void setUnknown(int modelID, quint16 unknown);
	const QList<QRgb> &lightColors(int modelID) const;
	void setLightColors(int modelID, const QList<QRgb> &lightColors);
	void setLightColor(int modelID, int colorID, QRgb lightColor);
	int animCount(int modelID) const;
	void insertAnim(int modelID, int numA, const QString &name);
	void removeAnim(int modelID, int numA);
	void swapAnim(int modelID, int oldNumA, int newNumA);
	const QStringList &ANames(int modelID) const;
	QString AName(int modelID, int numA=0) const;
	void setAName(int modelID, int numA, const QString &animName);
	quint16 animUnknown(int modelID, int numA) const;
	void setAnimUnknown(int modelID, int numA, quint16 unknown);
private:
	quint16 typeHRC;
	QStringList model_nameChar;
	QStringList model_nameHRC;
	QList<QStringList> model_anims;
	QList< QList<quint16> > model_anims_unknown;
	QList<quint16> model_unknown;
	QList<quint16> model_typeHRC;
	QList< QList<QRgb> > colors;
};

#endif // FIELDMODELLOADERPC_H

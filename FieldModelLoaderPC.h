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
	bool load(const QByteArray &data);
	QByteArray save() const;
	QString HRCName(int modelID) const;
	QString AName(int modelID, int numA=0) const;

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

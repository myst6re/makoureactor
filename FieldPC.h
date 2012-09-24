/****************************************************************************
 ** Makou Reactor Final Fantasy VII FieldPC Script Editor
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
#ifndef DEF_FIELDPC
#define DEF_FIELDPC

#include <QtGui>
#include "Field.h"
#include "FieldModelLoaderPC.h"
#include "FieldModelFilePC.h"

class FieldPC : public Field
{
public:
	FieldPC(quint32 position, const QString &name, FieldArchive *fieldArchive);
	FieldPC(const Field &field);
	virtual ~FieldPC();

	inline bool isPC() const { return true; }

	bool open(bool dontOptimize=false);
	QPixmap openBackground();
	QPixmap openBackground(const QHash<quint8, quint8> &paramActifs, const qint16 z[2], const bool *layers=NULL);

	bool getUsedParams(QHash<quint8, quint8> &usedParams, bool *layerExists);

	quint32 getPosition() const;
	void setPosition(quint32 position);

	bool save(QByteArray &newData, bool compress);
	qint8 importer(const QByteArray &data, bool isPSField, FieldParts part);

	FieldModelLoaderPC *getFieldModelLoader(bool open=true);
	FieldModelFilePC *getFieldModel(int modelID, int animationID=0, bool animate=true);
	FieldModelFilePC *getFieldModel(const QString &hrc, const QString &a, bool animate=true);
protected:
	QByteArray sectionData(FieldPart part);
	QByteArray sectionData(int idPart);
private:
	quint32 position;
	quint32 sectionPositions[9];
};

#endif // DEF_FIELDPC

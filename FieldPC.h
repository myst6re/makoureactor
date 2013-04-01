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
	FieldPC(const QString &name, FieldArchiveIO *fieldArchive);
	FieldPC(const Field &field);

	inline bool isPC() const { return true; }

	QPixmap openBackground(const QHash<quint8, quint8> &paramActifs, const qint16 z[2], const bool *layers=NULL);

	bool usedParams(QHash<quint8, quint8> &usedParams, bool *layerExists);

	bool save(QByteArray &newData, bool compress);
	qint8 importer(const QByteArray &data, bool isPSField, FieldParts part);

	FieldModelLoaderPC *fieldModelLoader(bool open=true);
	FieldModelFilePC *fieldModel(int modelID, int animationID=0, bool animate=true);
	FieldModelFilePC *fieldModel(const QString &hrc, const QString &a, bool animate=true);
protected:
	inline int headerSize() { return 42; }
	void openHeader(const QByteArray &fileData);
	FieldModelLoader *createFieldModelLoader();
	int sectionId(FieldPart part) const;
	quint32 sectionPosition(int idPart);
	inline int sectionCount() {	return 9; }
	inline int paddingBetweenSections() { return 4; }
private:
	quint32 sectionPositions[9];
};

#endif // DEF_FIELDPC

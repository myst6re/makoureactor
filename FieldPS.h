/****************************************************************************
 ** Makou Reactor Final Fantasy VII FieldPS Script Editor
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
#ifndef DEF_FIELDPS
#define DEF_FIELDPS

#include <QtGui>
#include "Field.h"
#include "FieldModelLoaderPS.h"
#include "FieldModelFilePS.h"
#include "FieldArchiveIO.h"

class FieldPS : public Field
{
public:
	FieldPS(const QString &name, FieldArchiveIO *io);
	FieldPS(const Field &field);

	inline bool isPC() const { return false; }

	QPixmap openBackground(const QHash<quint8, quint8> &paramActifs, const qint16 z[2], const bool *layers=NULL);

	bool save(QByteArray &newData, bool compress);

	FieldModelLoaderPS *fieldModelLoader(bool open=true);
	FieldModelFilePS *fieldModel(int modelID, int animationID=0, bool animate=true);
	FieldArchiveIOPS *io() const;
protected:
	inline int headerSize() { return 28; }
	void openHeader(const QByteArray &fileData);
	FieldModelLoader *createFieldModelLoader() const;
	BackgroundFile *createBackground() const;
	int sectionId(FieldPart part) const;
	quint32 sectionPosition(int idPart);
	inline int sectionCount() {	return 7; }
	inline int paddingBetweenSections() { return 0; }
private:
	quint32 sectionPositions[7];
};

#endif // DEF_FIELDPS

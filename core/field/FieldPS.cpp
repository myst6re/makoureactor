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
#include "FieldPS.h"
#include "BackgroundFilePS.h"
#include "../LZS.h"

FieldPS::FieldPS(const QString &name, FieldArchiveIO *io) :
      Field(name, io), vramDiff(0)
{
}

FieldPS::FieldPS(const Field &field) :
      Field(field), vramDiff(0)
{
}

FieldPS::~FieldPS()
{
	for (FieldModelFilePS *model: qAsConst(_models)) {
		if (model) {
			delete model;
		}
	}
}

void FieldPS::openHeader(const QByteArray &fileData)
{
	memcpy(sectionPositions, fileData.constData(), headerSize()); // header
	vramDiff = sectionPositions[0] - headerSize();// vram section1 pos - real section 1 pos

	for (quint8 i=0; i<7; ++i) {
		sectionPositions[i] -= vramDiff;
	}
}

int FieldPS::sectionId(FieldSection part) const
{
	switch (part) {
	case Scripts:
	case Akaos:			return 0;
	case Walkmesh:		return 1;
	case Background:
	case Tiles:			return 2;
	case Camera:		return 3;
	case Inf:			return 4;
	case Encounter:		return 5;
	case ModelLoader:	return 6;
	default:			return -1;
	}
}

quint32 FieldPS::sectionPosition(int idPart) const
{
	return sectionPositions[idPart];
}

FieldArchiveIOPS *FieldPS::io() const
{
	return static_cast<FieldArchiveIOPS *>(Field::io());
}

FieldPart *FieldPS::createPart(FieldSection part)
{
	switch (part) {
	case ModelLoader:	return new FieldModelLoaderPS(this);
	case Background:	return new BackgroundFilePS(this);
	default:			return Field::createPart(part);
	}
}

FieldModelLoaderPS *FieldPS::fieldModelLoader(bool open)
{
	return static_cast<FieldModelLoaderPS *>(Field::fieldModelLoader(open));
}

FieldModelFilePS *FieldPS::fieldModel(int modelID, int animationID, bool animate, bool open)
{
	_models.resize(modelID + 1);

	if (_models[modelID] == nullptr) {
		_models[modelID] = new FieldModelFilePS();
	}

	if (open) {
		int i = 0;
		for (FieldModelFilePS *model: qAsConst(_models)) {
			if (model != nullptr && !model->isModified()) {
				if (i == modelID) {
					model->load(this, modelID, animationID, animate);
				} else {
					model->clear();
				}
			}
			++i;
		}
	}

	return _models[modelID];
}

QByteArray FieldPS::saveHeader() const
{
	// No header
	return QByteArray();
}

QByteArray FieldPS::saveFooter() const
{
	// No footer
	return QByteArray();
}

QList<Field::FieldSection> FieldPS::orderOfSections() const
{
	return QList<FieldSection>() << Scripts << Walkmesh << Background << Camera << Inf << Encounter << ModelLoader;
}

bool FieldPS::isDatModified() const
{
	return true;
}

bool FieldPS::isBsxModified() const
{
	for (FieldModelFilePS *model: qAsConst(_models)) {
		if (model && model->isModified()) {
			return true;
		}
	}

	return false;
}

bool FieldPS::isMimModified() const
{
	return false;
}

bool FieldPS::saveModels(QByteArray &newData, bool compress)
{
	newData = QByteArray();

	if (!isOpen()) {
		qWarning() << "FieldPS::saveModels field is not open";
		return false;
	}

	newData = io()->modelData(this);
	if (newData.isEmpty()) {
		qWarning() << "FieldPS::saveModels cannot open bsx file";
		return false;
	}
	QBuffer ioBsx;
	ioBsx.setData(newData);
	if (!ioBsx.open(QIODevice::ReadWrite)) {
		qWarning() << "FieldPS::saveModels cannot open bsx buffer" << ioBsx.errorString();
		return false;
	}

	BsxFile bsx(&ioBsx);

	int modelID = 0;
	for (FieldModelFilePS *model: qAsConst(_models)) {
		if (model && model->isModified()) {
			if (!bsx.seek(modelID)) {
				qWarning() << "FieldPS::saveModels cannot seek to model";
				return false;
			}
			if (!bsx.writeModelHeader(*model)) {
				qWarning() << "FieldPS::saveModels cannot write model header";
				return false;
			}
		}
		++modelID;
	}

	newData = ioBsx.data();

	if (compress) {
		const QByteArray &compresse = LZS::compress(newData);
		quint32 lzsSize = compresse.size();
		newData = QByteArray((char *)&lzsSize, 4).append(compresse);
	}

	return true;
}

qint8 FieldPS::saveModels(const QString &path, bool compress)
{
	QByteArray newData;

	if (saveModels(newData, compress)) {
		QFile fic(path);
		if (!fic.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
			return 2;
		}
		fic.write(newData);
		fic.close();

		return 0;
	}

	return 1;
}

bool FieldPS::saveBackground(QByteArray &newData, bool compress)
{
	newData = QByteArray();

	if (!isOpen()) {
		qWarning() << "FieldPS::saveBackground field is not open";
		return false;
	}

	newData = io()->mimData(this, !compress);

	return true;
}

qint8 FieldPS::saveBackground(const QString &path, bool compress)
{
	QByteArray newData;

	if (saveBackground(newData, compress)) {
		QFile fic(path);
		if (!fic.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
			return 2;
		}
		fic.write(newData);
		fic.close();

		return 0;
	}

	return 1;
}

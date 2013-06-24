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
#include "FieldModelLoaderPC.h"
#include "Field.h"

FieldModelLoaderPC::FieldModelLoaderPC(Field *field) :
	FieldModelLoader(field)
{
}

void FieldModelLoaderPC::clear()
{
	model_nameChar.clear();
	model_nameHRC.clear();
	model_typeHRC.clear();
	model_anims.clear();
	this->model_unknown.clear();
	model_anims_unknown.clear();
	this->colors.clear();
}

void FieldModelLoaderPC::clean()
{
	for(int modelID=0 ; modelID<model_anims.size() ; ++modelID) {
		if(!model_nameChar.at(modelID).isEmpty()) {
			setModified(true);
			model_nameChar[modelID].clear();
		}

		const QStringList &animNames = model_anims.at(modelID);
		for(int animID=0 ; animID<animNames.size() ; ++animID) {
			const QString &animName = animNames.at(animID);
			int index = animName.lastIndexOf('.');
			if(index != -1) {
				model_anims[modelID][animID] = animName.left(index);
				setModified(true);
			}
		}
	}
}

bool FieldModelLoaderPC::open()
{
	return open(field()->sectionData(Field::ModelLoader));
}

bool FieldModelLoaderPC::open(const QByteArray &data)
{
	const char *constData = data.constData();

	if((quint32)data.size() < 6)	return false;

	quint32 i, j;
	quint16 nb;

	quint16 nbAnim, len, model_unknown;

	clear();

	memcpy(&nb, constData + 2, 2);
	memcpy(&typeHRC, constData + 4, 2);
	quint32 curPos = 6;
	for(i=0 ; i<nb ; ++i) {
		if((quint32)data.size() < curPos+2) {
			clear();
			return false;
		}

		memcpy(&len, constData + curPos, 2); // model name len

		if((quint32)data.size() < curPos+48+len) {
			clear();
			return false;
		}
		 // model name
		model_nameChar.append(QString(data.mid(curPos+2, len)));
		memcpy(&model_unknown, constData + curPos+2+len, 2);
		this->model_unknown.append(model_unknown);
		model_nameHRC.append(QString(data.mid(curPos+4+len,8)));
		bool ok;
		int typeHRC_value = QString(data.mid(curPos+12+len,4)).toInt(&ok);
		if(!ok)	typeHRC_value = typeHRC;
		model_typeHRC.append(typeHRC_value);

		memcpy(&nbAnim, constData + curPos+16+len, 2);

		QList<QRgb> color;

		for(j=0 ; j<10 ; ++j) {
			color.append(qRgb(data.at(curPos+18+len+j*3), data.at(curPos+19+len+j*3), data.at(curPos+20+len+j*3)));
		}
		this->colors.append(color);

		curPos += 48+len;

		QStringList anims;
		QList<quint16> anims_unknown;
		for(j=0 ; j<nbAnim ; ++j) {
			if((quint32)data.size() < curPos+2) {
				clear();
				return false;
			}

			memcpy(&len, constData + curPos, 2); // animation name len

			if((quint32)data.size() < curPos+4+len) {
				clear();
				return false;
			}

			anims.append(QString(data.mid(curPos+2, len))); // animation name

			memcpy(&model_unknown, constData + curPos+2+len, 2);
			anims_unknown.append(model_unknown);

			curPos += 4+len;
		}
		model_anims.append(anims);
		model_anims_unknown.append(anims_unknown);
	}
	setOpen(true);

	return true;
}

QByteArray FieldModelLoaderPC::save() const
{
	quint16 nbHRC = this->model_nameHRC.size(), nbAnim, nameSize, i, j;
	QByteArray HRCs;
	QString modelName;
	QRgb color;

	for(i=0 ; i<nbHRC ; ++i)
	{
		modelName = this->model_nameChar.at(i);
		nameSize = modelName.size();
		HRCs.append((char *)&nameSize, 2); //model name size
		HRCs.append(modelName.toLocal8Bit()); //model Name (fieldnamename_of_char.char)
		HRCs.append((char *)&this->model_unknown.at(i), 2); //Unknown
		HRCs.append(this->model_nameHRC.at(i).leftJustified(8, '\x00', true)); //HRC name (AAAA.HRC)
		HRCs.append(QString::number(this->model_typeHRC.at(i)).leftJustified(4, '\x00', true)); //scale (512 )
		nbAnim = this->model_anims.at(i).size();
		HRCs.append((char *)&nbAnim, 2); //Nb Anims
		for(j=0 ; j<10 ; ++j) //Colors
		{
			color = this->colors.at(i).at(j);
			HRCs.append((char)qRed(color));
			HRCs.append((char)qGreen(color));
			HRCs.append((char)qBlue(color));
		}
		for(j=0 ; j<nbAnim ; ++j) //Animations
		{
			nameSize = this->model_anims.at(i).at(j).size();
			HRCs.append((char *)&nameSize, 2); //Animation name size
			HRCs.append(this->model_anims.at(i).at(j)); //Animation name
			HRCs.append((char *)&model_anims_unknown.at(i).at(j), 2); //Animation unknown
		}
	}

	return QByteArray("\x00\x00", 2) //0
			.append((char *)&nbHRC, 2) //NbHRC
			.append((char *)&this->typeHRC, 2) //TypeHRC
			.append(HRCs);
}

quint16 FieldModelLoaderPC::globalScale() const
{
	return typeHRC;
}

void FieldModelLoaderPC::setGlobalScale(quint16 scale)
{
	if(typeHRC != scale) {
		typeHRC = scale;
		setModified(true);
	}
}

int FieldModelLoaderPC::modelCount() const
{
	return model_nameHRC.size();
}

bool FieldModelLoaderPC::insertModel(int modelID, const QString &hrcName)
{
	if(modelCount() < maxModelCount()) {
		QList<QRgb> color;
		if(!colors.isEmpty())
			color = colors.first();

		model_unknown.insert(modelID, (quint16)0);
		model_nameChar.insert(modelID, QString());
		model_nameHRC.insert(modelID, hrcName);
		model_typeHRC.insert(modelID, typeHRC);
		colors.insert(modelID, color);

		model_anims.insert(modelID, QStringList());
		model_anims_unknown.insert(modelID, QList<quint16>());

		setModified(true);
		return true;
	}
	return false;
}

void FieldModelLoaderPC::removeModel(int modelID)
{
	if(modelID >= 0 && modelID < modelCount()) {
		model_unknown.removeAt(modelID);
		model_nameChar.removeAt(modelID);
		model_nameHRC.removeAt(modelID);
		model_typeHRC.removeAt(modelID);
		colors.removeAt(modelID);

		model_anims.removeAt(modelID);
		model_anims_unknown.removeAt(modelID);

		setModified(true);
	}
}

void FieldModelLoaderPC::swapModel(int oldModelID, int newModelID)
{
	if(oldModelID != newModelID
			&& oldModelID >= 0 && oldModelID < modelCount()
			&& newModelID >= 0 && newModelID < modelCount()) {
		model_unknown.swap(oldModelID, newModelID);
		model_nameChar.swap(oldModelID, newModelID);
		model_nameHRC.swap(oldModelID, newModelID);
		model_typeHRC.swap(oldModelID, newModelID);
		colors.swap(oldModelID, newModelID);

		model_anims.swap(oldModelID, newModelID);
		model_anims_unknown.swap(oldModelID, newModelID);

		setModified(true);
	}
}

const QStringList &FieldModelLoaderPC::HRCNames() const
{
	return model_nameHRC;
}

QString FieldModelLoaderPC::HRCName(int modelID) const
{
	return model_nameHRC.value(modelID);
}

void FieldModelLoaderPC::setHRCName(int modelID, const QString &HRCName)
{
	if(modelID >= 0 && modelID < model_nameHRC.size()
			&& model_nameHRC.at(modelID) != HRCName) {
		model_nameHRC.replace(modelID, HRCName);
		setModified(true);
	}
}

const QStringList &FieldModelLoaderPC::charNames() const
{
	return model_nameChar;
}

QString FieldModelLoaderPC::charName(int modelID) const
{
	return model_nameChar.value(modelID);
}

void FieldModelLoaderPC::setCharName(int modelID, const QString &charName)
{
	if(modelID >= 0 && modelID < model_nameChar.size()
			&& model_nameChar.at(modelID) != charName) {
		model_nameChar.replace(modelID, charName);
		setModified(true);
	}
}

quint16 FieldModelLoaderPC::scale(int modelID) const
{
	return model_typeHRC.value(modelID);
}

void FieldModelLoaderPC::setScale(int modelID, quint16 scale)
{
	if(modelID >= 0 && modelID < model_typeHRC.size()
			&& model_typeHRC.at(modelID) != scale) {
		model_typeHRC.replace(modelID, scale);
		setModified(true);
	}
}

quint16 FieldModelLoaderPC::unknown(int modelID) const
{
	return model_unknown.value(modelID);
}

void FieldModelLoaderPC::setUnknown(int modelID, quint16 unknown)
{
	if(modelID >= 0 && modelID < model_unknown.size()
			&& model_unknown.at(modelID) != unknown) {
		model_unknown.replace(modelID, unknown);
		setModified(true);
	}
}

const QList<QRgb> &FieldModelLoaderPC::lightColors(int modelID) const
{
	return colors.at(modelID);
}

void FieldModelLoaderPC::setLightColors(int modelID, const QList<QRgb> &lightColors)
{
	if(modelID >= 0 && modelID < colors.size()
			&& colors.at(modelID) != lightColors) {
		colors.replace(modelID, lightColors);
		setModified(true);
	}
}

void FieldModelLoaderPC::setLightColor(int modelID, int colorID, QRgb lightColor)
{
	if(modelID >= 0 && modelID < colors.size()
			&& colorID < colors.at(modelID).size()
			&& colors.at(modelID).at(colorID) != lightColor) {
		colors[modelID].replace(colorID, lightColor);
		setModified(true);
	}
}

int FieldModelLoaderPC::animCount(int modelID) const
{
	if(modelID >= 0 && modelID < model_anims.size())
		return model_anims.at(modelID).size();
	return 0;
}

bool FieldModelLoaderPC::insertAnim(int modelID, int numA, const QString &name)
{
	if(modelID >= 0 && modelID < modelCount() && animCount(modelID) < maxAnimCount()) {
		model_anims[modelID].insert(numA, name);
		model_anims_unknown[modelID].insert(numA, 1);
		setModified(true);
		return true;
	}
	return false;
}

void FieldModelLoaderPC::removeAnim(int modelID, int numA)
{
	if(modelID >= 0 && modelID < modelCount()
			&& numA >= 0 && numA < animCount(modelID)) {
		model_anims[modelID].removeAt(numA);
		model_anims_unknown[modelID].removeAt(numA);
		setModified(true);
	}
}

void FieldModelLoaderPC::swapAnim(int modelID, int oldNumA, int newNumA)
{
	if(modelID >= 0 && modelID < modelCount()
			&& oldNumA != newNumA
			&& oldNumA >= 0 && oldNumA < animCount(modelID)
			&& newNumA >= 0 && newNumA < animCount(modelID)) {
		model_anims[modelID].swap(oldNumA, newNumA);
		model_anims_unknown[modelID].swap(oldNumA, newNumA);
		setModified(true);
	}
}

const QStringList &FieldModelLoaderPC::ANames(int modelID) const
{
	return model_anims.at(modelID);
}

QString FieldModelLoaderPC::AName(int modelID, int numA) const
{
	return model_anims.value(modelID).value(numA);
}

void FieldModelLoaderPC::setAName(int modelID, int numA, const QString &animName)
{
	if(modelID >= 0 && modelID < model_anims.size()) {
		QStringList &animNames = model_anims[modelID];
		if(numA >= 0 && numA < animNames.size()
				&& animNames.at(numA) != animName) {
			animNames.replace(numA, animName);
			setModified(true);
		}
	}
}

quint16 FieldModelLoaderPC::animUnknown(int modelID, int numA) const
{
	if(modelID < model_anims_unknown.size())
		return model_anims_unknown.at(modelID).value(numA);
	else
		return 0;
}

void FieldModelLoaderPC::setAnimUnknown(int modelID, int numA, quint16 unknown)
{
	if(modelID >= 0 && modelID < model_anims_unknown.size()) {
		QList<quint16> &animUnknown = model_anims_unknown[modelID];
		if(numA >= 0 && numA < animUnknown.size()
				&& animUnknown.at(numA) != unknown) {
			animUnknown.replace(numA, unknown);
			setModified(true);
		}
	}
}

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
	model_global_color.clear();
}

void FieldModelLoaderPC::clean()
{
	for (int modelID=0; modelID<model_anims.size(); ++modelID) {
		if (!model_nameChar.at(modelID).isEmpty()) {
			setModified(true);
			model_nameChar[modelID].clear();
		}

		const QStringList &animNames = model_anims.at(modelID);
		for (int animID=0; animID<animNames.size(); ++animID) {
			const QString &animName = animNames.at(animID);
			qsizetype index = animName.lastIndexOf('.');
			if (index != -1) {
				model_anims[modelID][animID] = animName.left(index);
				setModified(true);
			}
		}
	}
}

void FieldModelLoaderPC::initEmpty()
{
	model_nameChar.clear();
	model_nameChar.append("");
	model_nameChar.append("");
	model_nameChar.append("");

	model_nameHRC.clear();
	model_nameHRC.append("AAAA.HRC");
	model_nameHRC.append("AAGB.HRC");
	model_nameHRC.append("ABDA.HRC");

	model_anims.clear();
	model_anims.append(QStringList()
	                   << "AAFE"
	                   << "AAFF"
	                   << "AAGA");
	model_anims.append(QStringList()
	                   << "ABCD"
	                   << "ABCE"
	                   << "ABCF");
	model_anims.append(QStringList()
	                   << "ABIE"
	                   << "ABIF"
	                   << "ABJA");

	QList<quint16> animUnknown = QVector<quint16>(3, 1).toList();
	model_anims_unknown.clear();
	model_anims_unknown.append(animUnknown);
	model_anims_unknown.append(animUnknown);
	model_anims_unknown.append(animUnknown);

	model_unknown.clear();
	model_unknown.append(0);
	model_unknown.append(0);
	model_unknown.append(0);

	model_typeHRC.clear();
	model_typeHRC.append(512);
	model_typeHRC.append(512);
	model_typeHRC.append(512);

	QList<FieldModelColorDir> c;
	c
	    << FieldModelColorDir(-659, 411, 4034, qRgb(128, 128, 128))
	    << FieldModelColorDir(1536, -3349, -1839, qRgb(204, 204, 204))
	    << FieldModelColorDir(-2920, -1474, -2484, qRgb(77, 77, 77));

	colors.clear();
	colors.append(c);
	colors.append(c);
	colors.append(c);

	QRgb gc = qRgb(64, 64, 64);

	model_global_color.clear();
	model_global_color.append(gc);
	model_global_color.append(gc);
	model_global_color.append(gc);
}

bool FieldModelLoaderPC::open()
{
	return open(field()->sectionData(Field::ModelLoader));
}

bool FieldModelLoaderPC::open(const QByteArray &data)
{
	const char *constData = data.constData();

	if (quint32(data.size()) < 6) {
		qWarning() << "FieldModelLoaderPC::open data size < 6" << data.size();
		return false;
	}

	quint32 i, j;
	quint16 nb;

	quint16 nbAnim, len, model_unknown;

	clear();

	memcpy(&nb, constData + 2, 2);

	quint16 fieldScale = field()->scriptsAndTexts()->scale();

	quint32 curPos = 6;
	for (i = 0; i < nb; ++i) {
		if (quint32(data.size()) < curPos+2) {
			clear();
			qWarning() << "FieldModelLoaderPC::open data size < curPos+2" << data.size() << curPos;
			return false;
		}

		memcpy(&len, constData + curPos, 2); // model name len

		if (quint32(data.size()) < curPos+48+len) {
			clear();
			qWarning() << "FieldModelLoaderPC::open data size < curPos+48+len" << data.size() << curPos << len;
			return false;
		}
		 // model name
		const char *nameChar = constData + curPos + 2;
		// Trim \0 at the end
		model_nameChar.append(QString::fromLatin1(nameChar, qsizetype(qstrnlen(nameChar, len))));
		memcpy(&model_unknown, constData + curPos + 2 + len, 2);
		this->model_unknown.append(model_unknown);
		const char *nameHRC = constData + curPos + 4 + len;
		// Trim \0 at the end
		model_nameHRC.append(QString::fromLatin1(nameHRC, qsizetype(qstrnlen(nameHRC, 8))));
		bool ok;
		const char *typeValue = constData + curPos + 12 + len;
		// Trim \0 at the end
		quint16 typeHRC_value = quint16(QString::fromLatin1(typeValue, qsizetype(qstrnlen(typeValue, 4))).toUInt(&ok));
		if (!ok) {
			typeHRC_value = fieldScale;
		}
		model_typeHRC.append(typeHRC_value);

		memcpy(&nbAnim, constData + curPos+16+len, 2);

		QList<FieldModelColorDir> color;

		curPos += 18 + len;

		for (j=0; j<3; ++j) {
			qint16 dirA, dirB, dirC;
			memcpy(&dirA, constData + curPos + 3 + j * 9, 2);
			memcpy(&dirB, constData + curPos + 5 + j * 9, 2);
			memcpy(&dirC, constData + curPos + 7 + j * 9, 2);
			color.append(FieldModelColorDir(dirA, dirB, dirC,
			                                qRgb(data.at(curPos + 0 + j * 9),
			                                     data.at(curPos + 1 + j * 9),
			                                     data.at(curPos + 2 + j * 9))));
		}

		this->colors.append(color);

		model_global_color.append(qRgb(data.at(curPos + 0 + 27),
		                               data.at(curPos + 1 + 27),
		                               data.at(curPos + 2 + 27)));

		curPos += 30;

		QStringList anims;
		QList<quint16> anims_unknown;
		for (j=0; j<nbAnim; ++j) {
			if (quint32(data.size()) < curPos+2) {
				clear();
				qWarning() << "FieldModelLoaderPC::open data size < curPos+2 (2)" << data.size() << curPos;
				return false;
			}

			memcpy(&len, constData + curPos, 2); // animation name len

			if (quint32(data.size()) < curPos+4+len) {
				clear();
				qWarning() << "FieldModelLoaderPC::open data size < curPos+4+len" << data.size() << curPos << len;
				return false;
			}

			const char *anim = constData + curPos + 2;
			// Trim \0 at the end
			anims.append(QString::fromLatin1(anim, qsizetype(qstrnlen(anim, len)))); // animation name

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
	quint16 nbHRC = quint16(this->model_nameHRC.size());
	QByteArray HRCs;

	for (int i = 0; i < nbHRC; ++i) {
		const QString &modelName = this->model_nameChar.at(i);
		quint16 nameSize = quint16(modelName.size());
		HRCs.append((char *)&nameSize, 2); //model name size
		HRCs.append(modelName.toLocal8Bit()); //model Name (fieldnamename_of_char.char)
		HRCs.append((char *)&this->model_unknown.at(i), 2); //Unknown
		HRCs.append(this->model_nameHRC.at(i).toLatin1().leftJustified(8, '\x00', true)); //HRC name (AAAA.HRC)
		HRCs.append(QString::number(this->model_typeHRC.at(i)).toLatin1().leftJustified(4, '\x00', true)); //scale (512 )
		const quint16 nbAnim = quint16(this->model_anims.at(i).size());
		HRCs.append((char *)&nbAnim, 2); //Nb Anims

		for (quint8 j = 0; j < 3; ++j) { //Colors
			const FieldModelColorDir &colorDir = this->colors.at(i).at(j);
			HRCs.append(char(qRed(colorDir.color)));
			HRCs.append(char(qGreen(colorDir.color)));
			HRCs.append(char(qBlue(colorDir.color)));
			HRCs.append((char *)&colorDir.dirA, 2);
			HRCs.append((char *)&colorDir.dirB, 2);
			HRCs.append((char *)&colorDir.dirC, 2);
		}

		const QRgb &globalColor = this->model_global_color.at(i);
		HRCs.append(char(qRed(globalColor)));
		HRCs.append(char(qGreen(globalColor)));
		HRCs.append(char(qBlue(globalColor)));

		for (int j = 0; j < nbAnim; ++j) { //Animations
			nameSize = quint16(this->model_anims.at(i).at(j).size());
			HRCs.append((char *)&nameSize, 2); //Animation name size
			HRCs.append(this->model_anims.at(i).at(j).toLatin1()); //Animation name
			HRCs.append((char *)&model_anims_unknown.at(i).at(j), 2); //Animation unknown
		}
	}

	quint16 fieldScale = field()->scriptsAndTexts()->scale();

	return QByteArray("\x00\x00", 2)
			.append((char *)&nbHRC, 2)
			.append((char *)&fieldScale, 2)
			.append(HRCs);
}

qsizetype FieldModelLoaderPC::modelCount() const
{
	return model_nameHRC.size();
}

bool FieldModelLoaderPC::insertModel(int modelID, const QString &hrcName)
{
	if (modelCount() < maxModelCount()) {
		QList<FieldModelColorDir> color;
		if (!colors.isEmpty()) {
			color = colors.first();
		} else {
			color = QVector<FieldModelColorDir>(10, FieldModelColorDir()).toList();
		}
		QRgb globalColor;
		if (!model_global_color.isEmpty()) {
			globalColor = model_global_color.first();
		} else {
			globalColor = Qt::black;
		}

		model_unknown.insert(modelID, quint16(0));
		model_nameChar.insert(modelID, QString());
		model_nameHRC.insert(modelID, hrcName);
		model_typeHRC.insert(modelID, field()->scriptsAndTexts()->scale());
		colors.insert(modelID, color);
		model_global_color.insert(modelID, globalColor);

		model_anims.insert(modelID, QStringList());
		model_anims_unknown.insert(modelID, QList<quint16>());

		setModified(true);
		return true;
	}
	return false;
}

void FieldModelLoaderPC::removeModel(int modelID)
{
	if (modelID >= 0 && modelID < modelCount()) {
		model_unknown.removeAt(modelID);
		model_nameChar.removeAt(modelID);
		model_nameHRC.removeAt(modelID);
		model_typeHRC.removeAt(modelID);
		colors.removeAt(modelID);
		model_global_color.removeAt(modelID);

		model_anims.removeAt(modelID);
		model_anims_unknown.removeAt(modelID);

		setModified(true);
	}
}

void FieldModelLoaderPC::swapModel(int oldModelID, int newModelID)
{
	if (oldModelID != newModelID
			&& oldModelID >= 0 && oldModelID < modelCount()
			&& newModelID >= 0 && newModelID < modelCount()) {
		model_unknown.swapItemsAt(oldModelID, newModelID);
		model_nameChar.swapItemsAt(oldModelID, newModelID);
		model_nameHRC.swapItemsAt(oldModelID, newModelID);
		model_typeHRC.swapItemsAt(oldModelID, newModelID);
		colors.swapItemsAt(oldModelID, newModelID);
		model_global_color.swapItemsAt(oldModelID, newModelID);

		model_anims.swapItemsAt(oldModelID, newModelID);
		model_anims_unknown.swapItemsAt(oldModelID, newModelID);

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
	if (modelID >= 0 && modelID < model_nameHRC.size()
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
	if (modelID >= 0 && modelID < model_nameChar.size()
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
	if (modelID >= 0 && modelID < model_typeHRC.size()
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
	if (modelID >= 0 && modelID < model_unknown.size()
			&& model_unknown.at(modelID) != unknown) {
		model_unknown.replace(modelID, unknown);
		setModified(true);
	}
}

const QList<FieldModelColorDir> &FieldModelLoaderPC::lightColors(int modelID) const
{
	return colors.at(modelID);
}

void FieldModelLoaderPC::setLightColors(int modelID,
                                        const QList<FieldModelColorDir> &lightColors)
{
	if (modelID >= 0 && modelID < colors.size()
			&& colors.at(modelID) != lightColors) {
		colors.replace(modelID, lightColors);
		setModified(true);
	}
}

void FieldModelLoaderPC::setLightColor(int modelID, int colorID,
                                       const FieldModelColorDir &lightColor)
{
	if (modelID >= 0 && modelID < colors.size()
			&& colorID < colors.at(modelID).size()
			&& colors.at(modelID).at(colorID) != lightColor) {
		colors[modelID].replace(colorID, lightColor);
		setModified(true);
	}
}

QRgb FieldModelLoaderPC::globalColor(int modelID) const
{
	return model_global_color.at(modelID);
}

void FieldModelLoaderPC::setGlobalColor(int modelID, QRgb globalColor)
{
	if (modelID >= 0 && modelID < model_global_color.size()
			&& model_global_color.at(modelID) != globalColor) {
		model_global_color.replace(modelID, globalColor);
		setModified(true);
	}
}

qsizetype FieldModelLoaderPC::animCount(int modelID) const
{
	if (modelID >= 0 && modelID < model_anims.size()) {
		return model_anims.at(modelID).size();
	}
	return 0;
}

bool FieldModelLoaderPC::insertAnim(int modelID, int numA, const QString &name)
{
	if (modelID >= 0 && modelID < modelCount() && animCount(modelID) < maxAnimCount()) {
		model_anims[modelID].insert(numA, name);
		model_anims_unknown[modelID].insert(numA, 1);
		setModified(true);
		return true;
	}
	return false;
}

void FieldModelLoaderPC::removeAnim(int modelID, int numA)
{
	if (modelID >= 0 && modelID < modelCount()
			&& numA >= 0 && numA < animCount(modelID)) {
		model_anims[modelID].removeAt(numA);
		model_anims_unknown[modelID].removeAt(numA);
		setModified(true);
	}
}

void FieldModelLoaderPC::swapAnim(int modelID, int oldNumA, int newNumA)
{
	if (modelID >= 0 && modelID < modelCount()
			&& oldNumA != newNumA
			&& oldNumA >= 0 && oldNumA < animCount(modelID)
			&& newNumA >= 0 && newNumA < animCount(modelID)) {
		model_anims[modelID].swapItemsAt(oldNumA, newNumA);
		model_anims_unknown[modelID].swapItemsAt(oldNumA, newNumA);
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
	if (modelID >= 0 && modelID < model_anims.size()) {
		QStringList &animNames = model_anims[modelID];
		if (numA >= 0 && numA < animNames.size()
				&& animNames.at(numA) != animName) {
			animNames.replace(numA, animName);
			setModified(true);
		}
	}
}

quint16 FieldModelLoaderPC::animUnknown(int modelID, int numA) const
{
	if (modelID < model_anims_unknown.size()) {
		return model_anims_unknown.at(modelID).value(numA);
	}
	return 0;
}

void FieldModelLoaderPC::setAnimUnknown(int modelID, int numA, quint16 unknown)
{
	if (modelID >= 0 && modelID < model_anims_unknown.size()) {
		QList<quint16> &animUnknown = model_anims_unknown[modelID];
		if (numA >= 0 && numA < animUnknown.size()
				&& animUnknown.at(numA) != unknown) {
			animUnknown.replace(numA, unknown);
			setModified(true);
		}
	}
}

FieldModelInfosPC FieldModelLoaderPC::modelInfos(int modelID) const
{
	FieldModelInfosPC ret;
	ret.nameChar = model_nameChar.at(modelID);
	ret.nameHRC = model_nameHRC.at(modelID);
	ret.anims = model_anims.at(modelID);
	ret.animsUnknown = model_anims_unknown.at(modelID);
	ret.unknown = model_unknown.at(modelID);
	ret.typeHRC = model_typeHRC.at(modelID);
	ret.colors = colors.at(modelID);
	ret.globalColor = model_global_color.at(modelID);
	return ret;
}

void FieldModelLoaderPC::setModelInfos(int modelID,
                                       const FieldModelInfosPC &modelInfos)
{
	model_nameChar.replace(modelID, modelInfos.nameChar);
	model_nameHRC.replace(modelID, modelInfos.nameHRC);
	model_anims.replace(modelID, modelInfos.anims);
	model_anims_unknown.replace(modelID, modelInfos.animsUnknown);
	model_unknown.replace(modelID, modelInfos.unknown);
	model_typeHRC.replace(modelID, modelInfos.typeHRC);
	colors.replace(modelID, modelInfos.colors);
	model_global_color.replace(modelID, modelInfos.globalColor);
	setModified(true);
}

void FieldModelLoaderPC::insertModelInfos(int modelID,
                                          const FieldModelInfosPC &modelInfos)
{
	model_nameChar.insert(modelID, modelInfos.nameChar);
	model_nameHRC.insert(modelID, modelInfos.nameHRC);
	model_anims.insert(modelID, modelInfos.anims);
	model_anims_unknown.insert(modelID, modelInfos.animsUnknown);
	model_unknown.insert(modelID, modelInfos.unknown);
	model_typeHRC.insert(modelID, modelInfos.typeHRC);
	colors.insert(modelID, modelInfos.colors);
	model_global_color.insert(modelID, modelInfos.globalColor);
	setModified(true);
}

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
#include "FieldModelLoader.h"

FieldModelLoader::FieldModelLoader()
	: loaded(false), modified(false)
{
}

bool FieldModelLoader::isLoaded() const
{
	return loaded;
}

bool FieldModelLoader::isModified() const
{
	return modified;
}

void FieldModelLoader::setModified(bool modified)
{
	this->modified = modified;
}

void FieldModelLoader::load(const QByteArray &contenu, const QString &name)
{
	const char *constContenu = contenu.constData();

	quint32 posSection3, debutSection4;
	memcpy(&posSection3, &constContenu[14], 4);//Adresse Section 3
	memcpy(&debutSection4, &constContenu[18], 4);//Adresse Section 4

	if((quint32)contenu.size() < debutSection4)	return;

	quint32 i, j;
	quint16 nb;

	quint16 nbAnim, len, model_unknown;
	model_nameChar.clear();
	model_nameHRC.clear();
	model_typeHRC.clear();
	model_anims.clear();
	this->model_unknown.clear();
	this->colors.clear();

	memcpy(&nb, &constContenu[posSection3+6], 2);
	memcpy(&typeHRC, &constContenu[posSection3+8], 2);
	posSection3 += 10;
	for(i=0 ; i<nb ; ++i)
	{
		memcpy(&len, &constContenu[posSection3], 2);
		model_nameChar.append(QString(contenu.mid(posSection3+2+name.size(),len-name.size())));
		memcpy(&model_unknown, &constContenu[posSection3+2+len], 2);
		this->model_unknown.append(model_unknown);
		model_nameHRC.append(QString(contenu.mid(posSection3+4+len,8)));
		bool ok;
		int typeHRC_value = QString(contenu.mid(posSection3+12+len,4)).toInt(&ok);
		if(!ok)	typeHRC_value = typeHRC;
		model_typeHRC.append(typeHRC_value);

		memcpy(&nbAnim, &constContenu[posSection3+16+len], 2);

		QList<QRgb> color;

		for(j=0 ; j<10 ; ++j) {
			color.append(qRgb(contenu.at(posSection3+18+len+j*3), contenu.at(posSection3+19+len+j*3), contenu.at(posSection3+20+len+j*3)));
		}
		this->colors.append(color);

		posSection3 += 48+len;

		QStringList anims;
		for(j=0 ; j<nbAnim ; ++j) {
			memcpy(&len, &constContenu[posSection3], 2);
			anims.append(QString(contenu.mid(posSection3+2, len)));
			posSection3 += 4+len;
		}
		model_anims.append(anims);
	}
	loaded = true;
}

QByteArray FieldModelLoader::save(const QByteArray &contenu, const QString &name) const
{
	if(!loaded || !modified)	return contenu;

	quint16 nbHRC = this->model_nameHRC.size(), nbAnim, nameSize, i, j;
	QByteArray HRCs;
	QString modelName;
	QRgb color;

	for(i=0 ; i<nbHRC ; ++i)
	{
		modelName = name % this->model_nameChar.at(i);
		nameSize = modelName.size();
		HRCs.append((char *)&nameSize, 2); //model name size
		HRCs.append(modelName.toLocal8Bit()); //model Name (fieldnamename_of_char.char)
		HRCs.append((char *)&this->model_unknown.at(i), 2); //Unknown
		HRCs.append(this->model_nameHRC.at(i) % QString().setNum(this->model_typeHRC.at(i)).leftJustified(4, '\x00', true)); //HRC name (AAAA.HRC512 )
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
			HRCs.append("\x01\x00", 2); //1
		}
	}

	return QByteArray("\x00\x00", 2) //0
			.append((char *)&nbHRC, 2) //NbHRC
			.append((char *)&this->typeHRC, 2) //TypeHRC
			.append(HRCs);
}

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

FieldModelLoaderPC::FieldModelLoaderPC() :
	FieldModelLoader()
{
}

bool FieldModelLoaderPC::load(const QByteArray &data, const QString &name)
{
	const char *constData = data.constData();

	if((quint32)data.size() < 6)	return false;

	quint32 i, j;
	quint16 nb;

	quint16 nbAnim, len, model_unknown;
	model_nameChar.clear();
	model_nameHRC.clear();
	model_typeHRC.clear();
	model_anims.clear();
	this->model_unknown.clear();
	this->colors.clear();

	memcpy(&nb, &constData[2], 2);
	memcpy(&typeHRC, &constData[4], 2);
	quint32 curPos = 6;
	for(i=0 ; i<nb ; ++i)
	{
		if((quint32)data.size() < curPos+2)			return false;

		memcpy(&len, &constData[curPos], 2); // model name len

		if((quint32)data.size() < curPos+48+len)	return false;
		 // model name
		model_nameChar.append(QString(data.mid(curPos+2+name.size(),len-name.size())));
		memcpy(&model_unknown, &constData[curPos+2+len], 2);
		this->model_unknown.append(model_unknown);
		model_nameHRC.append(QString(data.mid(curPos+4+len,8)));
		bool ok;
		int typeHRC_value = QString(data.mid(curPos+12+len,4)).toInt(&ok);
		if(!ok)	typeHRC_value = typeHRC;
		model_typeHRC.append(typeHRC_value);

		memcpy(&nbAnim, &constData[curPos+16+len], 2);

		QList<QRgb> color;

		for(j=0 ; j<10 ; ++j) {
			color.append(qRgb(data.at(curPos+18+len+j*3), data.at(curPos+19+len+j*3), data.at(curPos+20+len+j*3)));
		}
		this->colors.append(color);

		curPos += 48+len;

		QStringList anims;
		for(j=0 ; j<nbAnim ; ++j) {
			if((quint32)data.size() < curPos+2)		return false;

			memcpy(&len, &constData[curPos], 2); // animation name len

			if((quint32)data.size() < curPos+4+len)	return false;

			anims.append(QString(data.mid(curPos+2, len))); // animation name
			curPos += 4+len;
		}
		model_anims.append(anims);
	}
	loaded = true;
	return true;
}

QByteArray FieldModelLoaderPC::save(const QByteArray &data, const QString &name) const
{
	if(!loaded || !modified)	return data;

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

QString FieldModelLoaderPC::HRCName(int modelID) const
{
	return model_nameHRC.value(modelID);
}

QString FieldModelLoaderPC::AName(int modelID, int numA) const
{
	return model_anims.value(modelID).value(numA);
}

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
#include "FieldModelFile.h"

FieldModelFile::FieldModelFile()
	: a_bones_count(0), dataLoaded(false)
{
}

FieldModelFile::~FieldModelFile()
{
	foreach(FieldModelPart *part, parts)
		delete part;
}

void FieldModelFile::clear()
{
	dataLoaded = false;
	rsd_files.clear();
	foreach(FieldModelPart *part, parts)
		delete part;
	parts.clear();
	tex_files.clear();
	loaded_tex.clear();
	tex2id.clear();
	bones.clear();
	frames.clear();
}

bool FieldModelFile::isLoaded() const
{
	return dataLoaded;
}

quint8 FieldModelFile::load(QString hrc, QString a, bool animate)
{
	if(hrc.isEmpty() || a.isEmpty()) {
		return 2;
	}

	QString charPath = Data::charlgp_path();
	if(charPath.isEmpty())	return 2;

	QFile fic(charPath);
	if(fic.open(QIODevice::ReadOnly))
	{
		Data::charlgp_loadListPos(&fic);

		int index;
		if((index=hrc.lastIndexOf('.')) != -1)
			hrc.truncate(index);
		if((index=a.lastIndexOf('.')) != -1)
			a.truncate(index);

		hrc = hrc.toLower();
		a = a.toLower();

		QString p;
		qint32 boneID;

		clear();

		int pos;
		if((pos=Data::charlgp_listPos.value(hrc%".hrc"))>0 && fic.seek(pos+20)
				&& open_hrc(&fic))
		{
			foreach(const QStringList &Ps, rsd_files) {
				foreach(QString rsd, Ps) {
					boneID = rsd_files.key(Ps);
					rsd = rsd.toLower();
					if((pos=Data::charlgp_listPos.value(rsd%".rsd"))>0 && fic.seek(pos+20)) {
						p = open_rsd(&fic, boneID);
						if(!p.isNull()) {
							FieldModelPartPC *part = new FieldModelPartPC();

							if((pos=Data::charlgp_listPos.value(p%".p"))>0 && fic.seek(pos+24)
									&& part->open(&fic)) {
								parts.insert(boneID, part);
							} else {
								delete part;
							}
						}
					}
				}
			}
			rsd_files.clear();

			if(!parts.isEmpty()
					&& (pos=Data::charlgp_listPos.value(a%".a"))>0 && fic.seek(pos+20) && open_a(&fic, animate))
			{
				QList<QList<int> > values = tex_files.values();

				foreach(const QList<int> &texs, values) {
					foreach(const int &tex, texs) {
						if(!loaded_tex.contains(tex) && (pos=Data::charlgp_listPos.value(tex2id.at(tex)%".tex"))>0 && fic.seek(pos+20)) {
							loaded_tex.insert(tex, open_tex(&fic));
						}
					}
				}

				tex2id.clear();

				this->dataLoaded = true;
			}
		}
	}
	else {
		return 2;
	}

	return this->dataLoaded;
}

bool FieldModelFile::open_hrc(QFile *hrc_file)
{
	bool ok;
	quint32 boneID=0, fileSize;

	hrc_file->read((char *)&fileSize, 4);
	fileSize += hrc_file->pos();

	QString line;
	quint32 nb_bones=0;
	do {
		line = QString(hrc_file->readLine()).trimmed();
		if(line.startsWith(QString(":BONES "))) {
			nb_bones = line.mid(7).toUInt(&ok);
			if(!ok) return false;
			if(nb_bones==0)	nb_bones = 1;//Null HRC fix
			break;
		}
	} while(hrc_file->pos()<fileSize);

	if(nb_bones==0)	return false;

	int nbP, lineType=0;
	Bone bone;
	bone.parent = 0;
	QMap<QString, int> nameToId;
	QStringList rsdlist;
	nameToId.insert("root", -1);

	while(hrc_file->pos()<fileSize && boneID<nb_bones) {
		QCoreApplication::processEvents();
		line = QString(hrc_file->readLine(fileSize-hrc_file->pos()+1)).trimmed();
		if(line.isEmpty() || line.startsWith(QChar('#')))
			continue;

		switch(lineType) {
		case 0: //Current
			nameToId.insert(line, boneID);
			break;
		case 1: //Parent
			bone.parent = nameToId.value(line, -1);
			break;
		case 2: //Length
			bone.size = line.toFloat(&ok);
			if(!ok) return false;
			bones.append(bone);
			break;
		case 3:
			rsdlist = line.split(' ', QString::SkipEmptyParts);
			if(rsdlist.size()<1) return false;
			nbP = rsdlist.first().toUInt(&ok);
			if(ok && nbP!=0) {
				rsdlist.removeFirst();
				if(rsdlist.size()==nbP) {
					this->rsd_files.insert(boneID, rsdlist);
				}
			}
			++boneID;
			break;
		}
		lineType = (lineType+1)%4;
	}

	return boneID==nb_bones;
}

bool FieldModelFile::open_a(QFile *a_file, bool animate)
{
	a_header header;
	VertexPC coords;
	quint32 fileSize, i, j;

	a_file->read((char *)&fileSize, 4);
	fileSize += a_file->pos();

	if(a_file->read((char *)&header, 36)!=36
	   || header.frames_count==0 || a_file->pos()+header.frames_count*(24+12*header.bones_count)>fileSize)
		return false;

	//qDebug() << header.bones_count << this->bones.size();
	//qDebug() << header.frames_count << fileSize-a_file->pos();

	this->a_bones_count = qMin((int)header.bones_count, this->bones.size());

	if(!animate)	header.frames_count = qMin(header.frames_count, (quint32)1);

	for(i=0 ; i<header.frames_count ; ++i)
	{
		if(!a_file->seek(a_file->pos()+24))	return false;

		QList<VertexPC> rotation_coords;

		for(j=0 ; j<header.bones_count ; ++j)
		{
			if(a_file->read((char *)&coords, 12)!=12)	return false;
			rotation_coords.append(coords);
		}
		this->frames.insert(i, rotation_coords);
	}
	return true;
}

QString FieldModelFile::open_rsd(QFile *rsd_file, int boneID)
{
	QString line, pname, tex;
	QList<int> texFiles;
	quint32 nTex=0, fileSize, i;
	int index;
	bool ok;

	rsd_file->read((char *)&fileSize, 4);
	fileSize += rsd_file->pos();

	while(rsd_file->pos()<fileSize)
	{
		line = QString(rsd_file->readLine(fileSize-rsd_file->pos()+1)).trimmed();
		if(pname.isNull() && (line.startsWith(QString("PLY=")) || line.startsWith(QString("MAT=")) || line.startsWith(QString("GRP="))))
		{
			if((index=line.lastIndexOf('.')) != -1)
				line.truncate(index);
			pname = line.mid(4).toLower();
		}
		else if(!pname.isNull() && nTex==0 && line.startsWith(QString("NTEX=")))
		{
			nTex = line.mid(5).toUInt(&ok);
			if(!ok) return QString();

			for(i=0 ; i<nTex && rsd_file->pos()<fileSize ; ++i)
			{
				line = QString(rsd_file->readLine(fileSize-rsd_file->pos()+1)).trimmed();
				if(!line.startsWith(QString("TEX[%1]=").arg(i))) return QString();

				if((index=line.lastIndexOf('.'))!=-1)
					line.truncate(index);
				tex = line.mid(line.indexOf('=')+1).toLower();
				if((index=tex2id.indexOf(tex)) != -1) {
					texFiles.append(index);
				} else {
					tex2id.append(tex);
					texFiles.append(tex2id.size()-1);
				}
			}
		}
	}
	tex_files.insert(boneID, texFiles);

	return pname;
}

QPixmap FieldModelFile::open_tex(QFile *tex_file)
{
	quint32 fileSize, l, h, nbPal, entreesPal, bitPerPx;

	tex_file->read((char *)&fileSize, 4);
	if(fileSize<236)	return QPixmap();

	tex_file->seek(tex_file->pos()+48);
	tex_file->read((char *)&nbPal, 4);
	tex_file->read((char *)&entreesPal, 4);
	tex_file->seek(tex_file->pos()+4);
	tex_file->read((char *)&l, 4);
	tex_file->read((char *)&h, 4);
	tex_file->seek(tex_file->pos()+36);
	tex_file->read((char *)&bitPerPx, 4);
	tex_file->seek(tex_file->pos()+128);

	QImage tex(l, h, QImage::Format_ARGB32);
	QRgb *pixels = (QRgb *)tex.bits();

	int size = l*h*bitPerPx, i;
	quint32 x=0, y=0;
	char imageData[size];

	if(nbPal > 0)
	{
		int sizePal = entreesPal*4*nbPal;
		char paletteData[sizePal];
		if(tex_file->read(paletteData, sizePal)==sizePal && tex_file->read(imageData, size)==size)
		{
			quint32 index;

			for(i=0 ; i<size ; ++i)
			{
				index = ((quint8)imageData[i])*4;
				pixels[x + y*l] = qRgba(paletteData[index+2], paletteData[index+1], paletteData[index], paletteData[index+3]);

				if(++x==l)
				{
					x = 0;
					++y;
				}
			}
		}
	}
	else
	{
		if(tex_file->read(imageData, size)==size)
		{
			quint16 color;

			for(i=0 ; i<size ; i+=2)
			{
				memcpy(&color, &imageData[i], 2);
				pixels[x + y*l] = PsColor::fromPsColor(color);

				if(++x==l)
				{
					x = 0;
					++y;
				}
			}
		}
	}
	return QPixmap::fromImage(tex);
}

quint8 FieldModelFile::load(const QByteArray &BSX_data, int model_id)
{
	const char *constData = BSX_data.constData();
	BSX_header header;
	Model_header model_header;
	Model model;

	memcpy(&header, constData, sizeof(BSX_header));
	qDebug() << "==== HEADER ====" << 0 << sizeof(BSX_header);
	qDebug() << "size" << header.size << "offset_models" << header.offset_models;

	if((quint32)BSX_data.size() != header.size || header.offset_models >= header.size) {
		qDebug() << BSX_data.size() << "error";
		return 2;
	}

	memcpy(&model_header, &constData[header.offset_models], sizeof(Model_header));

	qDebug() << "==== Model header ====" << header.offset_models << sizeof(Model_header);
	qDebug() << "num_models" << model_header.num_models << "vram" << QString::number(model_header.psx_memory, 16);
	qDebug() << "texPointer" << model_header.texture_pointer << "unknownPointer" << model_header.unknown_pointer;

	if(model_id >= (int)model_header.num_models) {
		qDebug() << "model_id too large" << model_id << model_header.num_models;
		return 2;
	}

	memcpy(&model, &constData[header.offset_models + sizeof(Model_header) + model_id*sizeof(Model)], sizeof(Model));

	qDebug() << "==== Model ====" << header.offset_models+sizeof(Model_header) << sizeof(Model) << model_header.num_models*sizeof(Model);
	qDebug() << "modelID" << model.model_id << "modelScale" << model.scale << "offsetSkeleton" << model.offset_skeleton;
	qDebug() << "r" << model.r1 << "g" << model.g1 << "b" << model.b1;
	qDebug() << "???" << model.unknown1;
	qDebug() << "???" << model.unknown2[0] << model.unknown2[1] << model.unknown2[2] << model.unknown2[3] << model.unknown2[4] << model.unknown2[5];
	qDebug() << "???" << model.unknown3;
	qDebug() << "r" << model.r2 << "g" << model.g2 << "b" << model.b2;
	qDebug() << "bones index" << model.index_bones << "count" << model.num_bones;
	qDebug() << "???" << model.unknown4[0] << model.unknown4[1] << model.unknown4[2] << model.unknown4[3] << model.unknown4[4] << model.unknown4[5];
	qDebug() << "???" << model.unknown5;
	qDebug() << "r" << model.r3 << "g" << model.g3 << "b" << model.b3;
	qDebug() << "parts index" << model.index_parts << "count" << model.num_parts;
	qDebug() << "???" << model.unknown6[0] << model.unknown6[1] << model.unknown6[2] << model.unknown6[3] << model.unknown6[4] << model.unknown6[5];
	qDebug() << "???" << model.unknown7;
	qDebug() << "r" << model.r4 << "g" << model.g4 << "b" << model.b4;
	qDebug() << "animations index" << model.index_animations << "count" << model.num_animations;

	int curOff = header.offset_models + sizeof(Model_header) + model_id*sizeof(Model) + model.offset_skeleton;
	BonePS bone;
	for(quint8 i=0 ; i<model.num_bones ; ++i) {
		memcpy(&bone, &constData[curOff+i*4], sizeof(BonePS));
		qDebug() << "bone" << i << bone.length << bone.parent << bone.unknown;
	}

	curOff += model.num_bones*4;

	for(quint8 i=0 ; i<model.num_parts ; ++i) {
		FieldModelPartPS *part = new FieldModelPartPS();
		qDebug() << "==== PART" << i << "====";
		if(part->open(constData, curOff, BSX_data.size())) {
			parts.insert(part->boneID(), part);
		} else {
			qWarning() << "Error open part" << i;
			delete part;
		}
		curOff += 32;
	}

/*
	PS
bone 0 0 0 255 0
bone 1 0 0 0 1
bone 2 255 203 1 1
bone 3 255 123 2 1
bone 4 255 203 1 0
bone 5 255 130 4 0
bone 6 255 186 5 1
bone 7 255 183 6 1
bone 8 255 116 7 1
bone 9 255 203 1 0
bone 10 255 130 9 0
bone 11 255 186 10 1
bone 12 255 183 11 1
bone 13 255 116 12 1
bone 14 0 0 0 0
bone 15 255 196 14 1
bone 16 255 79 15 1
bone 17 255 57 16 1
bone 18 0 0 0 0
bone 19 255 196 18 1
bone 20 255 79 19 1
bone 21 255 57 20 1
	PC
bone 0 -1 1.74572
bone 1 0 4.35991
bone 2 1 6.83339
bone 3 0 4.13482
bone 4 3 2.30646
bone 5 4 2.38332
bone 6 5 4.60649
bone 7 6 2.5994
bone 8 0 4.13482
bone 9 8 2.30646
bone 10 9 2.38332
bone 11 10 4.60649
bone 12 11 2.5994
bone 13 -1 1.96156
bone 14 13 5.83091
bone 15 14 6.5494
bone 16 15 6.90588
bone 17 -1 1.96156
bone 18 17 5.83091
bone 19 18 6.5494
bone 20 19 6.90588
=== reno ===
	PS
bone 0 0 0 255 0
bone 1 0 0 0 1
bone 2 255 191 1 1
bone 3 255 39 2 1
bone 4 255 23 3 0
bone 5 255 33 4 1
bone 6 255 191 1 0
bone 7 255 101 6 0
bone 8 255 201 7 1
bone 9 255 165 8 1
bone 10 255 129 9 1
bone 11 255 191 1 0
bone 12 255 101 11 0
bone 13 255 201 12 1
bone 14 255 165 13 1
bone 15 255 129 14 1
bone 16 0 0 0 0
bone 17 255 199 16 1
bone 18 255 66 17 1
bone 19 255 74 18 1
bone 20 0 0 0 0
bone 21 255 199 20 1
bone 22 255 66 21 1
bone 23 255 74 22 1
	PC
bone 0 -1 2.12653
bone 1 0 7.12961
bone 2 1 7.64647
bone 3 2 7.32801
bone 4 3 4.52066
bone 5 0 5.10938
bone 6 5 1.81955
bone 7 6 2.99726
bone 8 7 4.16469
bone 9 8 2.33435
bone 10 0 5.10938
bone 11 10 1.81955
bone 12 11 2.99726
bone 13 12 4.16469
bone 14 13 2.33435
bone 15 -1 1.86406
bone 16 15 6.23619
bone 17 16 5.96725
bone 18 17 5.22121
bone 19 -1 1.86406
bone 20 19 6.23619
bone 21 20 5.96725
bone 22 21 5.22121
*/

	curOff += model.num_bones*sizeof(BonePS);

	Part part;
	Vertex vertex;

	for(quint8 i=0 ; i<model.num_parts ; ++i) {
		memcpy(&part, &constData[curOff+i*sizeof(Part)], sizeof(Part));
		qDebug() << i << part.bone_index << part.numVertices << (part.offset_vertex & 0xFFFF) << (part.offset_vertex >> 16) << part.offset_poly << part.offset_control;
		memcpy(&vertex, &constData[(part.offset_vertex & 0xFFFF)+4], sizeof(Vertex));
		qDebug() << vertex.v.x << vertex.v.y << vertex.v.z << vertex.unknown;
	}

	return true;
}

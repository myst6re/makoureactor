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
#include "FieldModelFilePC.h"
#include "Data.h"
#include "Palette.h"
#include "FieldModelPartPC.h"

FieldModelFilePC::FieldModelFilePC() :
	FieldModelFile()
{
}

void FieldModelFilePC::clear()
{
	tex2id.clear();
	_tex_files.clear();

	FieldModelFile::clear();
}

quint8 FieldModelFilePC::load(QString hrc, QString a, bool animate)
{
	if(hrc.isEmpty() || a.isEmpty()) {
		return 2;
	}

	if(Data::charlgp_loadListPos())
	{
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

		QMultiMap<int, QStringList> rsd_files;
		QIODevice *hrcFile = Data::charLgp.file(hrc % ".hrc");

		if(hrcFile && hrcFile->open(QIODevice::ReadOnly)
				&& openHrc(hrcFile, rsd_files)) {
			foreach(const QStringList &Ps, rsd_files) {
				foreach(QString rsd, Ps) {
					boneID = rsd_files.key(Ps);
					rsd = rsd.toLower();

					QIODevice *rsdFile = Data::charLgp.file(rsd % ".rsd");

					if(rsdFile && rsdFile->open(QIODevice::ReadOnly)) {
						p = openRsd(rsdFile, boneID);
						if(!p.isNull()) {
							FieldModelPartPC *part = new FieldModelPartPC();

							QIODevice *pFile = Data::charLgp.file(p % ".p");

							if(pFile && pFile->open(QIODevice::ReadOnly)
									&& part->open(pFile)) {
								_parts.insert(boneID, part);

//								QFile textOut(QString("fieldModelPartPC%1.txt").arg(_parts.size()-1));
//								textOut.open(QIODevice::WriteOnly);
//								textOut.write(part->toString().toLatin1());
//								textOut.close();
							} else {
								delete part;
							}
						}
					}
				}
			}
			rsd_files.clear();

			QIODevice *aFile = Data::charLgp.file(a % ".a");

			if(!_parts.isEmpty()
					&& aFile && aFile->open(QIODevice::ReadOnly) && openA(aFile, animate))
			{
				// Open all loaded tex
				int texID=0;
				foreach(const QString &texName, tex2id) {
					QPixmap tex;
					QIODevice *texFile = Data::charLgp.file(texName % ".tex");
					if(texFile && texFile->open(QIODevice::ReadOnly)) {
						tex = openTex(texFile);
					}
					_loaded_tex.insert(texID, tex);
					++texID;
				}

				// Convert relative group tex IDs to absolute (relative to _loaded_tex) tex IDs
				QMapIterator<int, FieldModelPart *> it(_parts);
				while(it.hasNext()) {
					it.next();
					int boneID = it.key();
					QList<int> texs = _tex_files.value(boneID);
					foreach(FieldModelGroup *group, it.value()->groups()) {
						if(group->textureNumber() >= 0 && group->textureNumber() < texs.size()) {
							group->setTextureNumber(texs.at(group->textureNumber()));
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

bool FieldModelFilePC::openHrc(QIODevice *hrc_file, QMultiMap<int, QStringList> &rsd_files)
{
	bool ok;
	QString line;
	quint32 boneCount=0;

	do {
		line = QString(hrc_file->readLine()).trimmed();
		if(line.startsWith(QString(":BONES "))) {
			boneCount = line.mid(7).toUInt(&ok);
			if(!ok) return false;
			if(boneCount==0)	boneCount = 1;//Null HRC fix
			break;
		}
	} while(hrc_file->canReadLine());

	if(boneCount==0)	return false;

	int nbP, lineType=0;
	quint32 boneID=0;
	Bone bone;
	bone.parent = 0;
	QMap<QString, int> nameToId;
	QStringList rsdlist;
	nameToId.insert("root", -1);

	while(hrc_file->canReadLine() && boneID < boneCount) {
		QCoreApplication::processEvents();
		line = QString(hrc_file->readLine()).trimmed();
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
			bone.size = -line.toFloat(&ok);
			if(!ok) return false;
			_bones.append(bone);
			break;
		case 3: //RSD list
			rsdlist = line.split(' ', QString::SkipEmptyParts);
			if(rsdlist.size()<1) return false;
			nbP = rsdlist.first().toUInt(&ok);
			if(ok && nbP!=0) {
				rsdlist.removeFirst();
				if(rsdlist.size()==nbP) {
					rsd_files.insert(boneID, rsdlist);
				}
			}
			++boneID;
			break;
		}
		lineType = (lineType + 1) % 4;
	}

	return boneID==boneCount;
}

bool FieldModelFilePC::openA(QIODevice *a_file, bool animate)
{
	a_header header;
	PolyVertex rot/*, trans*/;

	if(a_file->read((char *)&header, 36) != 36
			|| header.frames_count == 0
			|| a_file->pos() + header.frames_count*(24+12*header.bones_count) > a_file->size())
		return false;

//	qDebug() << header.bones_count << _bones.size();
//	qDebug() << header.frames_count << a_file->size()-a_file->pos();

	this->a_bones_count = qMin((int)header.bones_count, _bones.size());

	if(!animate)	header.frames_count = qMin(header.frames_count, (quint32)1);

	for(quint32 i=0 ; i<header.frames_count ; ++i)
	{
		if(!a_file->seek(a_file->pos()+24))	return false;
//		if(a_file->read((char *)&trans, 12)!=12)	return false;

		QList<PolyVertex> rotation_coords;

		for(quint32 j=0 ; j<header.bones_count ; ++j)
		{
			if(a_file->read((char *)&rot, 12) != 12)	return false;
			rotation_coords.append(rot);
		}
		_frames.insert(i, rotation_coords);
//		_framesTrans.insert(i, QList<PolyVertex>() << trans);
	}
	return true;
}

QString FieldModelFilePC::openRsd(QIODevice *rsd_file, int boneID)
{
	QString line, pname, tex;
	QList<int> texIds;
	quint32 nTex=0, i;
	int index;
	bool ok;

	while(rsd_file->canReadLine())
	{
		line = QString(rsd_file->readLine()).trimmed();
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

			for(i=0 ; i<nTex && rsd_file->canReadLine() ; ++i)
			{
				line = QString(rsd_file->readLine()).trimmed();
				if(!line.startsWith(QString("TEX[%1]=").arg(i))) return QString();

				if((index=line.lastIndexOf('.'))!=-1)
					line.truncate(index);
				tex = line.mid(line.indexOf('=')+1).toLower();

				if((index=tex2id.indexOf(tex)) != -1) {
					texIds.append(index);
				} else {
					tex2id.append(tex);
					texIds.append(tex2id.size()-1);
				}
			}
		}
	}
	if(!texIds.isEmpty())
		_tex_files.insert(boneID, texIds);

	return pname;
}

QPixmap FieldModelFilePC::openTex(QIODevice *tex_file)
{
	quint32 l, h, nbPal, entreesPal, bitPerPx;

	if(tex_file->size() < 236)								return QPixmap();

	if(!tex_file->seek(tex_file->pos()+48))					return QPixmap();
	if(tex_file->read((char *)&nbPal, 4) != 4)				return QPixmap();
	if(tex_file->read((char *)&entreesPal, 4) != 4)			return QPixmap();
	if(!tex_file->seek(tex_file->pos()+4))					return QPixmap();
	if(tex_file->read((char *)&l, 4) != 4)					return QPixmap();
	if(tex_file->read((char *)&h, 4) != 4)					return QPixmap();
	if(!tex_file->seek(tex_file->pos()+36))					return QPixmap();
	if(tex_file->read((char *)&bitPerPx, 4) != 4)			return QPixmap();
	if(!tex_file->seek(tex_file->pos()+128))				return QPixmap();

	QImage tex(l, h, QImage::Format_ARGB32);
	QRgb *pixels = (QRgb *)tex.bits();

	int size = l*h*bitPerPx, i;
	quint32 x=0, y=0;
	char *imageData = new char[size];

	if(nbPal > 0)
	{
		if(bitPerPx != 1) {
			delete imageData;
			return QPixmap();
		}

		quint32 sizePal = nbPal > 0 ? entreesPal*4 : 0;
		char paletteData[sizePal];
		if(tex_file->read(paletteData, sizePal)==sizePal && tex_file->read(imageData, size)==size)
		{
			quint32 index;

			for(i=0 ; i<size ; ++i)
			{
				index = ((quint8)imageData[i])*4;
				if(index+3 >= sizePal) {
					delete imageData;
					return QPixmap();
				}
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
		if(bitPerPx != 2 && bitPerPx != 3 && bitPerPx != 4) {
			delete imageData;
			return QPixmap();
		}

		if(tex_file->read(imageData, size)==size)
		{
			quint16 color;

			for(i=0 ; i<size ; i+=bitPerPx)
			{
				if(bitPerPx == 2) {
					memcpy(&color, &imageData[i], 2);
					pixels[x + y*l] = PsColor::fromPsColor(color);
				} else if(bitPerPx == 3) {
					pixels[x + y*l] = qRgb(imageData[i+2], imageData[i+1], imageData[i]);
				} else if(bitPerPx == 4) {
					pixels[x + y*l] = qRgba(imageData[i+2], imageData[i+1], imageData[i], imageData[i+3]);
				}

				if(++x==l)
				{
					x = 0;
					++y;
				}
			}
		}
	}
	delete imageData;

	return QPixmap::fromImage(tex);
}

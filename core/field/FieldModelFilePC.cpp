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
#include "CharArchive.h"
#include "FieldModelPartIOPC.h"
#include "../TexFile.h"

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

quint8 FieldModelFilePC::load(const QString &hrc, const QString &a, bool animate)
{
	if(hrc.isEmpty() || a.isEmpty()) {
		return 2;
	}

	CharArchive *charLgp = CharArchive::instance();
	if(charLgp->isOpen()) {
		QString hrcFilename, aFilename;
		int index;
		if((index=hrc.lastIndexOf('.')) != -1)
			hrcFilename = hrc.left(index);
		else
			hrcFilename = hrc;
		if((index=a.lastIndexOf('.')) != -1)
			aFilename = a.left(index);
		else
			aFilename = a;

		QString p;
		qint32 boneID;

		clear();

		QMultiMap<int, QStringList> rsd_files;
		QIODevice *hrcFile = charLgp->fileIO(hrcFilename % ".hrc");

		if(hrcFile && hrcFile->open(QIODevice::ReadOnly)
				&& openHrc(hrcFile, rsd_files)) {
			foreach(const QStringList &Ps, rsd_files) {
				foreach(QString rsd, Ps) {
					boneID = rsd_files.key(Ps);
					rsd = rsd.toLower();

					QIODevice *rsdFile = charLgp->fileIO(rsd % ".rsd");

					if(rsdFile && rsdFile->open(QIODevice::ReadOnly)) {
						p = openRsd(rsdFile, boneID);
						if(!p.isNull()) {
							QIODevice *pFile = charLgp->fileIO(p % ".p");

							if(pFile && pFile->open(QIODevice::ReadOnly)) {
								FieldModelPartIOPC partIO(pFile);
								FieldModelPart *part = new FieldModelPart();
								if(partIO.read(part)) {
									_parts.insert(boneID, part);

//									QFile textOut(QString("fieldModelPartPC%1.txt").arg(_parts.size()-1));
//									textOut.open(QIODevice::WriteOnly);
//									textOut.write(part->toString().toLatin1());
//									textOut.close();
								} else {
									delete part;
								}
							}
						}
					}
				}
			}
			rsd_files.clear();

			QIODevice *aFile = charLgp->fileIO(aFilename % ".a");

			if(!_parts.isEmpty()
					&& aFile && aFile->open(QIODevice::ReadOnly) && openA(aFile, animate))
			{
				// Open all loaded tex
				int texID=0;
				foreach(const QString &texName, tex2id) {
					QImage tex;
					QIODevice *texFile = charLgp->fileIO(texName % ".tex");
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

				dataLoaded = true;
			}
		}
	}
	else {
		return 2;
	}

	return dataLoaded;
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
//		QCoreApplication::processEvents();
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
			bone.size = -line.toFloat(&ok) / MODEL_SCALE_PC;
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
	PolyVertex rot, trans;

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
		if(!a_file->seek(a_file->pos() + 12))		return false;
		if(a_file->read((char *)&trans, 12) != 12)	return false;
		trans.x = trans.x / MODEL_SCALE_PC;
		trans.y = trans.y / MODEL_SCALE_PC;
		trans.z = trans.z / MODEL_SCALE_PC;

		QList<PolyVertex> rotation_coords;

		for(quint32 j=0 ; j<header.bones_count ; ++j)
		{
			if(a_file->read((char *)&rot, 12) != 12)	return false;
			rotation_coords.append(rot);
		}
		_animation.insertFrame(i, rotation_coords, QList<PolyVertex>() << trans);
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

QImage FieldModelFilePC::openTex(QIODevice *tex_file)
{
	TexFile tex(tex_file->readAll());
	if(!tex.isValid()) {
		return QImage();
	}
	return tex.image();
}

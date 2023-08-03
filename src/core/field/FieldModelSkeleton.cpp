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
#include "FieldModelSkeleton.h"

FieldModelBone::FieldModelBone(float size, int parent,
							   const QList<FieldModelPart *> &parts) :
	_size(size), _parent(parent), _parts(parts)
{
}

QImage FieldModelBone::toImage(int width, int height) const
{
	if (_parts.isEmpty()) {
		return QImage();
	}

	QImage image(width, height, QImage::Format_ARGB32_Premultiplied);
	image.fill(Qt::black);

	QPainter p(&image);
	qreal tileH = height / _parts.size();

	for (int i = 0; i < int(_parts.size()); ++i) {
		p.drawImage(QRectF(0.0, i * tileH, width, tileH), _parts.at(i)->toImage(width * 2, qCeil(tileH) * 2));
	}

	return image;
}

int FieldModelBone::commonColorCount(const FieldModelBone &other) const
{
	int count = _parts.size();

	if (other.parts().size() != count) {
		qDebug() << "FieldModelBone::commonColorCount different part count" << other.parts().size() << count;
		
		return -1;
	}

	int ret = 0;

	for (int i = 0; i < count; ++i) {
		FieldModelPart *part = _parts.at(i),
		    *otherPart = other.parts().at(i);
		
		int common = part->commonColorCount(*otherPart);
		
		if (common < 0) {
			return -1;
		}

		ret += common;
	}

	return ret;
}

FieldModelSkeleton::FieldModelSkeleton()
{
}

FieldModelSkeleton::FieldModelSkeleton(const QList<FieldModelBone> &bones) :
	_bones(bones)
{
}

QString FieldModelSkeleton::toString() const
{
	QString ret;
	int boneID = 0;

	for (const FieldModelBone &bone : _bones) {
		ret.append(QString("Bone %1: parent= %2 size= %3\n")
				   .arg(boneID)
				   .arg(bone.parent())
				   .arg(int(bone.size())));
		++boneID;
	}

	return ret;
}

QImage FieldModelSkeleton::toImage(int width, int height) const
{
	if (_bones.isEmpty()) {
		return QImage();
	}

	QImage image(width, height, QImage::Format_ARGB32_Premultiplied);
	image.fill(Qt::black);

	QPainter p(&image);
	qreal tileW = width / _bones.size();

	for (int i = 0; i < int(_bones.size()); ++i) {
		p.drawImage(QRectF(i * tileW, 0.0, tileW, height),
		            _bones.at(i).toImage(qCeil(tileW) * 2, height * 2));
	}

	return image;
}

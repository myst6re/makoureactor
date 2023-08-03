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
#include "FieldModelFile.h"

FieldModelFile::FieldModelFile()
{
}

FieldModelFile::~FieldModelFile()
{
	for (const FieldModelBone &bone : _skeleton.bones()) {
		qDeleteAll(bone.parts());
	}
}

void FieldModelFile::clear()
{
	for (const FieldModelBone &bone : _skeleton.bones()) {
		qDeleteAll(bone.parts());
	}
	_skeleton.clear();
}

bool FieldModelFile::isValid() const
{
	return _skeleton.boneCount() == 1;
}

QByteArray FieldModelFile::signature() const
{
	quint32 boneCount = quint32(translateAfter() ? _skeleton.boneCount() : std::max(qsizetype(1), _skeleton.boneCount() - 1));
	QByteArray ret;

	ret.append(QString::number(boneCount, 16).rightJustified(4, QChar('0'), true).toLatin1());

	for (int i = translateAfter() ? 0 : 1; i < _skeleton.boneCount(); ++i) {
		const FieldModelBone &bone = _skeleton.bone(i);
		int parent = bone.parent();
		ret.append('B');
		ret.append(QString::number(translateAfter() ? parent : parent - 1, 16).rightJustified(4, QChar('0'), true).toLatin1());
		quint32 partCount = quint32(bone.parts().size());
		ret.append(QString::number(partCount, 16).rightJustified(4, QChar('0'), true).toLatin1());
		for (FieldModelPart *part: bone.parts()) {
			ret.append('P');
			quint32 groupCount = quint32(part->groups().size());
			ret.append(QString::number(groupCount, 16).rightJustified(4, QChar('0'), true).toLatin1());
			for (FieldModelGroup *group: part->groups()) {
				ret.append('G');
				ret.append("TEX", 3);
				ret.append(group->hasTexture() ? '1' : '0');
				/* QMap<QRgb, bool> colors;
				for (Poly *polygon: group->polygons()) {
					for (QRgb color: polygon->colors()) {
						colors.insert(color, true);
					}
				}
				QList<QRgb> cs = colors.keys();
				for (QRgb color: cs) {
					ret.append('C');
					ret.append(QString::number(qRed(color), 16).rightJustified(2, QChar('0'), true).toLatin1());
					ret.append(QString::number(qGreen(color), 16).rightJustified(2, QChar('0'), true).toLatin1());
					ret.append(QString::number(qBlue(color), 16).rightJustified(2, QChar('0'), true).toLatin1());
				} */
			}
		}
		
	}

	return ret;
}

int FieldModelFile::commonColorCount(const FieldModelFile &other) const
{
	quint32 boneCount = quint32(translateAfter() ? _skeleton.boneCount() : std::max(qsizetype(1), _skeleton.boneCount() - 1)),
	    otherBoneCount = quint32(other.translateAfter() ? other.skeleton().boneCount() : std::max(qsizetype(1), other.skeleton().boneCount() - 1)),
	    boneStart = translateAfter() ? 0 : 1,
	    otherBoneStart = other.translateAfter() ? 0 : 1;

	if (boneCount != otherBoneCount) {
		qDebug() << "FieldModelFile::commonColorCount different bone count" << boneCount << otherBoneCount;
		return -1;
	}
	
	int ret = 0;
	
	for (quint32 i = 0; i < boneCount; ++i) {
		const FieldModelBone &bone = _skeleton.bone(boneStart + i),
		    &otherBone = other.skeleton().bone(otherBoneStart + i);

		int common = bone.commonColorCount(otherBone);
		
		if (common < 0) {
			return -1;
		}
		
		ret += common;
	}
	
	return ret;
}

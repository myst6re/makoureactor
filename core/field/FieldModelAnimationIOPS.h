/****************************************************************************
 ** Makou Reactor Final Fantasy VII Field Script Editor
 ** Copyright (C) 2009-2015 Arzel Jérôme <myst6re@gmail.com>
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
#ifndef FIELDMODELANIMATIONIOPS_H
#define FIELDMODELANIMATIONIOPS_H

#include "FieldModelAnimationIO.h"
#include "FieldModelPartIOPS.h"

struct Animation {
	quint16 numFrames;                  // Number of frames
	quint8 numBones;                    // Number of bones
	quint8 numFramesTranslation;        // Number of translation frames
	quint8 numStaticTranslation;        // Number of static translation frames
	quint8 numFramesRotation;           // Number of rotation frames
	quint16 offsetFramesTranslation;    // Relative offset to translation frames
	quint16 offsetStaticTranslation;    // Relative offset to statis translation frames
	quint16 offsetFramesRotation;       // Relative offset to rotation frames
	quint32 offsetData;                 // Offset to animation data section
};

struct FrameTranslation {
	quint8 flag;
	quint8 rx, ry, rz;
	quint8 tx, ty, tz;
	quint8 padding;
};

class FieldModelAnimationIOPS : public FieldModelAnimationIO
{
public:
	explicit FieldModelAnimationIOPS(QIODevice *device);
	virtual ~FieldModelAnimationIOPS() {}

	bool read(FieldModelAnimation &animation, int maxFrames = -1) const;
	bool write(const FieldModelAnimation &animation) const;
};

#endif // FIELDMODELANIMATIONIOPS_H

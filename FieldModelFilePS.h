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
#ifndef FIELDMODELFILEPS_H
#define FIELDMODELFILEPS_H

#include <QtGui>
#include "FieldModelFile.h"
#include "FieldModelPartPS.h"
#include "FieldModelLoaderPS.h"
#include "TdbFile.h"

typedef struct {
	quint16 num_frames;					// Number of frames
	quint8 num_bones;					// Number of bones
	quint8 num_frames_translation;		// Number of translation frames
	quint8 num_static_translation;		// Number of static translation frames
	quint8 num_frames_rotation;			// Number of rotation frames
	quint16 offset_frames_translation;	// Relative offset to translation frames
	quint16 offset_static_translation;	// Relative offset to statis translation frames
	quint16 offset_frames_rotation;		// Relative offset to rotation frames
	quint32 offset_data;				// Offset to animation data section
} Animation;

typedef struct {
	quint16 width, height;
	quint16 vramX, vramY;
	quint32 offset_data;
} TexHeader;

typedef struct {
	quint8 flag;
	quint8 rx, ry, rz;
	quint8 tx, ty, tz;
	quint8 padding;
} FrameTranslation;

typedef struct {
	quint32 size;
	quint32 offset_models;
} BSX_header;

typedef struct {
	quint32 psx_memory;
	quint32 num_models;
	quint32 texture_pointer;
	quint32 unknown_pointer;
} Model_header;

typedef struct {
	quint8 red, green, blue;
} ColorRGB;

typedef struct {
	quint16 model_id;        // ID of the model
	quint16 scale;
	quint32 offset_skeleton; // Offset to the parts, bones and animations of the model
	quint8 r1, g1, b1;
	quint8 unknown;
	quint8 r2, g2, b2;
	quint8 r3, g3, b3;
	quint8 index_bones_start;
	quint8 index_bones_end;
	quint8 r4, g4, b4;
	quint8 num_bones;      // Number of bones in the model's skeleton
	quint8 r5, g5, b5;
	quint8 r6, g6, b6;
	quint8 index_parts_start;
	quint8 index_parts_end;
	quint8 r7, g7, b7;
	quint8 num_parts;      // Number of parts in the model's skeleton
	quint8 r8, g8, b8;
	quint8 r9, g9, b9;
	quint8 index_animations_start;
	quint8 index_animations_end;
	quint8 r10, g10, b10;
	quint8 num_animations; // Number of animations
} Model;

typedef struct {
	quint16 unknown;
	quint8 num_bones;
	quint8 num_parts;
	quint8 num_animations;
	quint8 blank1[17];
	quint16 scale;
	quint16 offset_parts; // relative to the end of this structure
	quint16 offset_animations; // relative to the end of this structure
	quint32 offset_skeleton;
	quint32 blank2;
} BCXModel;

typedef struct {
	qint16 length;
	qint8 parent;
	quint8 unknown;
} BonePS;

class FieldArchive;
class Field;

class FieldModelFilePS : public FieldModelFile
{
public:
	FieldModelFilePS();
	bool isPS() const { return true; }
	quint8 load(FieldArchive *fieldArchive, Field *currentField, int model_id, int animation_id, bool animate=false);

private:
	int openSkeleton(const char *constData, int curOff, int size, quint8 numBones);
	int openMesh(const char *constData, int curOff, int size, quint8 numParts);
	bool openAnimation(const char *constData, int curOff, int animation_id, int size, bool animate=false);
	QPixmap openTexture(const char *constData, int size, const TexHeader &imgHeader, const TexHeader &palHeader, quint8 bpp);
	bool openBCX(const QByteArray &BCX, int animationID, bool animation=false);
};

#endif // FIELDMODELFILEPS_H

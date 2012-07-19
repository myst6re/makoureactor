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
#ifndef FIELDMODELFILE_H
#define FIELDMODELFILE_H

#include <QtGui>
#include "FieldModelPart.h"
#include "Data.h"
#include "Palette.h"

typedef struct {
	quint32 version;
	quint32 frames_count;
	quint32 bones_count;
	quint8 rotation_order[3];
	quint8 unused;
	quint32 runtime_data[5];
} a_header;

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
	quint32 size;
	quint32 offset_models;
	quint8 unknown[4];
} BSX_header;

typedef struct {
	quint32 psx_memory;
	quint32 num_models;
	quint32 texture_pointer;
	quint32 unknown_pointer;
} Model_header;

typedef struct {
	quint16 model_id;        // ID of the model
	quint16 scale;
	quint32 offset_skeleton; // Offset to the parts, bones and animations of the model
	quint8 r1, g1, b1;
	quint8 unknown1;
	quint8 unknown2[6];
	quint8 index_bones;
	quint8 unknown3;
	quint8 r2, g2, b2;
	quint8 num_bones;      // Number of bones in the model's skeleton
	quint8 unknown4[6];
	quint8 index_parts;
	quint8 unknown5;
	quint8 r3, g3, b3;
	quint8 num_parts;      // Number of parts in the model's skeleton
	quint8 unknown6[6];
	quint8 index_animations;
	quint8 unknown7;
	quint8 r4, g4, b4;
	quint8 num_animations; // Number of animations
} Model;

typedef struct {
	float size;
	int parent;
} Bone;

typedef struct {
	quint16 size;
	quint8 parent;
	quint8 unknown;
} BonePS;

class FieldModelFile
{
public:
	FieldModelFile();
	bool isLoaded() const;
	quint8 load(QString hrc, QString a, bool animate=false);
	quint8 load(const QByteArray &BSX_data, int model_id);
	void clear();

	QMultiMap<int, FieldModelPart> P_files;
	QMultiMap<int, QList<int> > tex_files;
	QHash<int, QPixmap> loaded_tex;
	QList<Bone> bones;
	int a_bones_count;
	QHash<int, QList<VertexPC> > frames;
private:
	QMultiMap<int, QStringList> rsd_files;
	QList<QString> tex2id;

	bool open_hrc(QFile *);
	bool open_a(QFile *, bool animate=false);
	QString open_rsd(QFile *, int);
	static QPixmap open_tex(QFile *);

	bool dataLoaded;
};

#endif // FIELDMODELFILE_H

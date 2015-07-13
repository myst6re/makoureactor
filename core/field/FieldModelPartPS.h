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
/* Many thanks to Akari for his work. */
#ifndef FIELDMODELPARTPS_H
#define FIELDMODELPARTPS_H

#include <QtCore>
#include "FieldModelPart.h"

struct TextureInfo {
	quint8 type; // 0: eye, 1: mouth, 2: normal
	quint8 bpp;
	quint16 imgX, imgY;
	quint16 palX, palY;

	bool operator==(const struct TextureInfo &ti) const {
		return ti.bpp == bpp
				&& ti.imgX == imgX
				&& ti.imgY == imgY
				&& ti.type == type;
	}

	bool operator<(const struct TextureInfo &) const {
		return false;
	}
};

class FieldModelPartPS : public FieldModelPart
{
public:
	FieldModelPartPS();
	inline const QList<TextureInfo> &textures() const {
		return _textures;
	}
	inline void setTextures(const QList<TextureInfo> &textures) {
		_textures = textures;
	}
	void addTexturedPolygon(quint8 control, Poly *polygon);
	void addPolygon(Poly *polygon);
private:
	QList<TextureInfo> _textures;
};

#endif // FIELDMODELPARTPS_H

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

#include <QtCore>
#include <QImage>
#include "FieldModelFile.h"
#include "FieldModelTextureRefPS.h"
#include "FieldModelLoader.h"

class FieldPS;

class FieldModelFilePS : public FieldModelFile
{
public:
	FieldModelFilePS();
	void clear();
	inline bool translateAfter() const { return false; }
	inline const QList<FieldModelColorDir> &lightColors() const {
		return _colors;
	}
	inline QRgb globalColor() const {
		return _globalColor;
	}
	inline void setLightColors(const QList<FieldModelColorDir> &colors) {
		_colors = colors;
	}
	inline void setGlobalColor(QRgb color) {
		_globalColor = color;
	}
	inline quint16 scale() const {
		return _scale;
	}
	inline void setScale(quint16 scale) {
		_scale = scale;
	}
	bool load(FieldPS *currentField, int modelID, int animationID, bool animate);
	QImage loadedTexture(FieldModelGroup *group);
	inline void *textureIdForGroup(FieldModelGroup *group) const {
		return (void *)group;
	}
	QHash<void *, QImage> loadedTextures();
	QImage vramImage() const;
private:
	Q_DISABLE_COPY(FieldModelFilePS)
	QList<FieldModelColorDir> _colors;
	QRgb _globalColor;
	quint16 _scale;
	FieldPS *_currentField;
	int _currentModelID;
	QHash<FieldModelGroup *, QImage> _loadedTex;
	FieldModelTexturesPS _textures;
};

#endif // FIELDMODELFILEPS_H

/****************************************************************************
 ** Makou Reactor Final Fantasy VII Field Script Editor
 ** Copyright (C) 2009-2021 Arzel Jérôme <myst6re@gmail.com>
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
#pragma once

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
	inline bool isModified() const {
		return _isModified;
	}
	void clearModified() {
		_isModified = false;
	}
	inline bool translateAfter() const {
		return false;
	}
	inline const QList<FieldModelColorDir> &lightColors() const {
		return _colors;
	}
	inline void setLightColors(const QList<FieldModelColorDir> &colors) {
		if (colors != _colors) {
			_colors = colors;
			_isModified = true;
		}
	}
	inline void setLightColor(int id, const FieldModelColorDir &color) {
		if (_colors.at(id) != color) {
			_colors.replace(id, color);
			_isModified = true;
		}
	}
	inline QRgb globalColor() const {
		return _globalColor;
	}
	inline void setGlobalColor(QRgb color) {
		if (_globalColor != color) {
			_globalColor = color;
			_isModified = true;
		}
	}
	inline quint16 scale() const {
		return _scale;
	}
	inline void setScale(quint16 scale) {
		if (_scale != scale) {
			_scale = scale;
			_isModified = true;
		}
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
	FieldPS *_currentField;
	int _currentModelID;
	QHash<FieldModelGroup *, QImage> _loadedTex;
	FieldModelTexturesPS _textures;
	quint16 _scale;
	bool _isModified;
};

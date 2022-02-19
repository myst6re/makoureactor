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
/* Many thanks to Akari for his work. */
#pragma once

#include <QImage>
#include "FieldModelTextureRef.h"

/* struct TextureInfo {
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

	bool operator!=(const struct TextureInfo &ti) const {
		return !(*this == ti);
	}

	bool operator<(const struct TextureInfo &ti) const {
		return memcmp(this, &ti, sizeof(TextureInfo)) < 0;
	}
}; */

class FieldModelTexturesPS
{
public:
	enum Bpp {
		Bpp4 = 0,
		Bpp8 = 1
	};

	FieldModelTexturesPS() {}
	FieldModelTexturesPS(const QList<QRect> &rects,
						const QList<QByteArray> &data);
	virtual ~FieldModelTexturesPS();

	QImage toImage(const QPoint &imgPos, const QPoint &palPos, Bpp bpp) const;
	QImage toImage(const QPoint &palPos, Bpp bpp) const;
	inline QImage toImage(int imgX, int imgY, int palX, int palY, Bpp bpp) const {
		return toImage(QPoint(imgX, imgY), QPoint(palX, palY), bpp);
	}
	inline const QList<QRect> &rects() const {
		return _rects;
	}
	inline void setRects(const QList<QRect> rects) {
		_rects = rects;
	}
	inline void setData(const QList<QByteArray> data) {
		_data = data;
	}
private:
	QPair<int, int> resolve(const QPoint &imgPos, const QPoint &palPos) const;
	QImage toImage(int imgId, int palId, Bpp bpp) const;

	QList<QRect> _rects;
	QList<QByteArray> _data;
};

class FieldModelTextureRefPS : public FieldModelTextureRef
{
public:
	FieldModelTextureRefPS() :
		_type(0), _bpp(0),
		_imgX(0), _imgY(0),
		_palX(0), _palY(0) {}
	FieldModelTextureRefPS(quint8 type,
						   quint8 bpp,
						   quint16 imgX,
						   quint16 imgY,
						   quint16 palX,
						   quint16 palY) :
	_type(type), _bpp(bpp),
	_imgX(imgX), _imgY(imgY),
	_palX(palX), _palY(palY) {}
	virtual ~FieldModelTextureRefPS() override;

	inline quint8 type() const {
		return _type;
	}

	inline void setType(quint8 type) {
		_type = type;
	}

	inline quint8 bpp() const {
		return _bpp;
	}

	inline void setBpp(quint8 bpp) {
		_bpp = bpp;
	}

	inline quint16 imgX() const {
		return _imgX;
	}

	inline void setImgX(quint16 imgX) {
		_imgX = imgX;
	}

	inline quint16 imgY() const {
		return _imgY;
	}

	inline void setImgY(quint16 imgY) {
		_imgY = imgY;
	}

	inline QPoint imgPos() const {
		return QPoint(_imgX, _imgY);
	}

	inline quint16 palX() const {
		return _palX;
	}

	inline void setPalX(quint16 palX) {
		_palX = palX;
	}

	inline quint16 palY() const {
		return _palY;
	}

	inline void setPalY(quint16 palY) {
		_palY = palY;
	}

	inline quint64 textureIdentifier() const override {
		return quint16(((imgX() / 64) & 0x0F)
		               | (((imgY() / 256) & 0x01) << 4)
		               | (((palX() / 16) & 0x3F) << 5)
		               | ((palY() & 0x1FF) << 11)
		               | ((type() & 0x3F) << 20)
		               | ((bpp() & 0x3) << 26));
	}
private:
	quint8 _type; // 0: eye, 1: mouth, 2: normal
	quint8 _bpp;
	quint16 _imgX, _imgY;
	quint16 _palX, _palY;
};

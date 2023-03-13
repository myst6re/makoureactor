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
#include "FieldModelTextureRefPS.h"
#include <PsColor>
#include <QPixmap>
#include <QPainter>

FieldModelTexturesPS::FieldModelTexturesPS(const QList<QRect> &rects,
                                           const QList<QByteArray> &data) :
    _rects(rects), _data(data)
{
}

FieldModelTexturesPS::~FieldModelTexturesPS()
{
}

QImage FieldModelTexturesPS::toImage(const QPoint &imgPos, const QPoint &palPos, Bpp bpp) const
{
	QPair<int, int> ids = resolve(imgPos, palPos);
	if (ids.first < 0 || ids.second < 0) {
		return QImage();
	}

	return toImage(ids.first, ids.second, bpp);
}

QImage FieldModelTexturesPS::toImage(const QPoint &palPos, Bpp bpp) const
{
	QPixmap image(720, 512);
	QPainter p(&image);

	for (const QRect &rect : rects()) {
		if (rect.height() != 1) {
			p.drawImage(rect.topLeft(), toImage(rect.topLeft(), palPos, bpp));
		}
	}

	return image.toImage();
}

QPair<int, int> FieldModelTexturesPS::resolve(const QPoint &imgPos, const QPoint &palPos) const
{
	int imgId = -1,
	        palId = -1,
	        i = 0;

	for (const QRect &rect : _rects) {
		if (imgId < 0 && rect.topLeft() == imgPos) {
			imgId = i;
		} else if (palId < 0 && rect.topLeft() == palPos) {
			palId = i;
		}

		if (imgId >= 0 && palId >= 0) {
			break;
		}

		i++;
	}

	return qMakePair(imgId, palId);
}

QImage FieldModelTexturesPS::toImage(int imgId, int palId, Bpp bpp) const
{
	QRect imgRect = _rects.at(imgId);
	QByteArray imgData = _data.at(imgId),
			palData = _data.at(palId);
	const char *imgConstData = imgData.constData(),
			*palConstData = palData.constData();

	if (imgRect.width() * imgRect.height() * 2 > imgData.size()) {
		qWarning() << "FieldModelTexturePS::openTexture Offset texture too large";
		return QImage();
	}

	int width = imgRect.width();
	if (bpp == Bpp8) {
		width *= 2;
	} else/* if (bpp == Bpp4)*/ {
		width *= 4;
	}

	QImage image(width, imgRect.height(), QImage::Format_ARGB32);
	QRgb *px = reinterpret_cast<QRgb *>(image.bits());
	int i = 0;

	for (int y = 0; y < imgRect.height(); ++y) {
		for (int x = 0; x < imgRect.width() * 2; ++x) {
			quint8 index = quint8(imgConstData[i]);
			quint16 color;

			if (bpp == Bpp8) {
				if (index * 2 + 2 > palData.size()) {
					qWarning() << "FieldModelTexturePS::openTexture Offset palette too large";
					continue;
				}
				memcpy(&color, palConstData + index * 2, 2);

				px[i] = PsColor::fromPsColor(color, true);
			} else/* if (bpp == Bpp4)*/ {
				if ((index & 0xF) * 2 + 2 > palData.size()) {
					qWarning() << "FieldModelTexturePS::openTexture Offset palette too large";
					continue;
				}
				memcpy(&color, palConstData + (index & 0xF) * 2, 2);

				px[i * 2] = PsColor::fromPsColor(color, true);

				if ((index >> 4) * 2 + 2 > palData.size()) {
					qWarning() << "FieldModelTexturePS::openTexture Offset palette too large";
					continue;
				}
				memcpy(&color, palConstData + (index >> 4) * 2, 2);

				px[i * 2 + 1] = PsColor::fromPsColor(color, true);
			}
			++i;
		}
	}

	return image;
}

FieldModelTextureRefPS::~FieldModelTextureRefPS()
{
}

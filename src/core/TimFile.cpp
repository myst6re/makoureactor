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
#include "TimFile.h"
#include <PsColor.h>

TimFile::TimFile(const QByteArray &data) :
    TextureFile(),
    bpp(0),
    palX(0), palY(0),
    palW(0), palH(0),
    imgX(0), imgY(0)
{
	open(data);
}

TimFile::TimFile(const TextureFile &texture,
                 quint8 bpp,
                 quint16 palX, quint16 palY,
                 quint16 palW, quint16 palH,
                 quint16 imgX, quint16 imgY) :
    TextureFile(texture),
    bpp(bpp),
    palX(palX), palY(palY),
    palW(palW), palH(palH),
    imgX(imgX), imgY(imgY)
{
}

bool TimFile::open(const QByteArray &data)
{
	//	QTime t;t.start();

	quint32 palSize=0, color=0;
	quint16 w, h;
	const char *constData = data.constData();
	bool hasPal;
	int dataSize = data.size();

	if (!data.startsWith(QByteArray("\x10\x00\x00\x00", 4)) || dataSize < 8) {
		return false;
	}

//	quint8 tag = (quint8)data.at(0);
//	quint8 version = (quint8)data.at(1);
	bpp = (quint8)data.at(4) & 3;
	hasPal = ((quint8)data.at(4) >> 3) & 1;

//	qDebug() << QString("=== Apercu TIM ===");
//	qDebug() << QString("Tag = %1, version = %2, reste = %3").arg(tag).arg(version).arg(QString(data.mid(2,2).toHex()));
//	qDebug() << QString("bpp = %1, hasPal = %2, flag = %3, reste = %4").arg(bpp).arg(hasPal).arg((quint8)data.at(4),0,2).arg(QString(data.mid(5,3).toHex()));

	if (hasPal && bpp > 1)
		return false;

	_colorTables.clear();

	if (hasPal)
	{
		if (dataSize < 20)
			return false;

		memcpy(&palSize, &constData[8], 4);
		memcpy(&palX, &constData[12], 2);
		memcpy(&palY, &constData[14], 2);
		memcpy(&palW, &constData[16], 2);
		memcpy(&palH, &constData[18], 2);
		if ((quint32)dataSize < 8+palSize/* || palSize != (quint32)palW*palH*2+12*/)
			return false;

		quint16 onePalSize = (bpp==0 ? 16 : 256);
		int nbPal = (palSize-12)/(onePalSize*2);
		if ((palSize-12)%(onePalSize*2) != 0)
		{
			nbPal *= 2;
		}
		if (nbPal > 0)
		{
			int pos=0;
			for (int i=0; i<nbPal; ++i)
			{
				QVector<QRgb> pal;
				QBitArray alphaBits(onePalSize);

				for (quint16 j=0; j<onePalSize; ++j) {
					memcpy(&color, &constData[20+pos*2+j*2], 2);
					pal.append(PsColor::fromPsColor(color, true));
					alphaBits.setBit(j, psColorAlphaBit(color));
				}

				_colorTables.append(pal);
				_alphaBits.append(alphaBits);

				pos += pos % palW == 0 ? onePalSize : palW - onePalSize;
			}
		}
		else
		{
			return false;
		}

		_currentColorTable = 0;

//		quint16 palX, palY;
//		memcpy(&palX, &constData[12], 2);
//		memcpy(&palY, &constData[14], 2);

//		qDebug() << QString("-Palette-");
//		qDebug() << QString("Size = %1, w = %2, h = %3").arg(palSize).arg(palW).arg(palH);
//		qDebug() << QString("x = %1, y = %2").arg(palX).arg(palY);
//		qDebug() << QString("NbPal = %1 (valid : %2)").arg(nbPal).arg((palSize-12)%(onePalSize*2));
	}

	if ((quint32)dataSize < 20+palSize)
		return false;

	memcpy(&sizeImgSection, &constData[8+palSize], 4);
	memcpy(&imgX, &constData[12+palSize], 2);
	memcpy(&imgY, &constData[14+palSize], 2);
	memcpy(&w, &constData[16+palSize], 2);
	memcpy(&h, &constData[18+palSize], 2);
	if (bpp==0)		w*=4;
	else if (bpp==1)	w*=2;

//	qDebug() << QString("-Image-");
//	qDebug() << QString("Size = %1, w = %2, h = %3").arg(sizeImgSection).arg(w).arg(h);
//	qDebug() << QString("TIM Size = %1").arg(8+palSize+sizeImgSection);

	_image = QImage(w, h, hasPal ? QImage::Format_Indexed8 : QImage::Format_ARGB32);
	if (hasPal) {
		_image.setColorTable(_colorTables.first());
	}
	//_image.fill(QColor(0, 0, 0, 0));
	QRgb *pixels = (QRgb *)_image.bits();

	int size, i=0;
	quint32 x=0, y=0;

	if (bpp!=0) {
		size = qMin((quint32)(12 + w*h*bpp), dataSize - 8 - palSize);
	} else {
		size = qMin((quint32)(12 + w/2*h), dataSize - 8 - palSize);
	}

	if (8 + palSize + size > (quint32)dataSize)
		return false;

	if (bpp==0)//mag176, icon
	{
		while (i<size && y<h)
		{
			_image.setPixel(x, y, (quint8)data.at(20+palSize+i) & 0xF);
			++x;
			if (x==w)
			{
				x = 0;
				++y;
			}

			_image.setPixel(x, y, (quint8)data.at(20+palSize+i) >> 4);
			++x;
			if (x==w)
			{
				x = 0;
				++y;
			}
			++i;
		}
	}
	else if (bpp==1)
	{
		while (i<size && y<h)
		{
			_image.setPixel(x, y, (quint8)data.at(20+palSize+i));

			++x;
			if (x==w)
			{
				x = 0;
				++y;
			}
			++i;
		}
	}
	else if (bpp==2)
	{
		QBitArray alphaBits;

		while (i<size && y<h)
		{
			memcpy(&color, &constData[20+palSize+i], 2);
			pixels[x + y*w] = PsColor::fromPsColor(color, true);
			alphaBits.setBit(i / 2, psColorAlphaBit(color));

			++x;
			if (x==w)
			{
				x = 0;
				++y;
			}
			i+=2;
		}

		_alphaBits.append(alphaBits);
	}
	else if (bpp==3)
	{
		while (i<size && y<h)
		{
			memcpy(&color, &constData[20+palSize+i], 3);
			pixels[x + y*w] = qRgb(color >> 16, (color >> 8) & 0xFF, color & 0xFF);

			++x;
			if (x==w)
			{
				x = 0;
				++y;
			}
			i+=3;
		}
	}

//	qDebug() << t.elapsed();
	return true;
}

bool TimFile::save(QByteArray &data) const
{
	Q_ASSERT(_colorTables.size() == _alphaBits.size());

	bool hasPal = bpp <= 1;
	quint32 flag = (hasPal << 3) | (bpp & 3);

	// Header
	data.append("\x10\x00\x00\x00", 4);
	data.append((char *)&flag, 4);

	if (hasPal) {
		quint16 colorPerPal = bpp == 0 ? 16 : 256;
		quint32 sizePalSection = 12 + _colorTables.size() * colorPerPal * 2;

		data.append((char *)&sizePalSection, 4);
		data.append((char *)&palX, 2);
		data.append((char *)&palY, 2);
		data.append((char *)&palW, 2);
		data.append((char *)&palH, 2);

		int colorTableId = 0;
		for (const QVector<QRgb> &colorTable : _colorTables) {
			const QBitArray &alphaBit = _alphaBits.at(colorTableId);
			int i;

			Q_ASSERT(colorTable.size() == colorPerPal);
			Q_ASSERT(alphaBit.size() == colorPerPal);

			for (i=0; i<colorTable.size() && i<colorPerPal; ++i) {
				quint16 psColor = PsColor::toPsColor(colorTable.at(i));
				psColor = setPsColorAlphaBit(psColor, alphaBit.at(i));
				data.append((char *)&psColor, 2);
			}
			if (i<colorPerPal) {
				data.append(QByteArray(colorPerPal - i, '\0'));
			}

			++colorTableId;
		}

		quint16 width = _image.width(), height = _image.height();
		if (bpp==0) {
			width/=4;
		}
		else {
			width/=2;
		}

		data.append((char *)&sizeImgSection, 4);
		data.append((char *)&imgX, 2);
		data.append((char *)&imgY, 2);
		data.append((char *)&width, 2);
		data.append((char *)&height, 2);

		width *= 2;

		for (int y=0; y<height; ++y) {
			for (int x=0; x<width; ++x) {
				if (bpp == 0) {
					quint8 index = (_image.pixelIndex(x*2, y) & 0xF) | ((_image.pixelIndex(x*2+1, y) & 0xF) << 4);
					data.append((char)index);
				} else {
					data.append((char)_image.pixelIndex(x, y));
				}
			}
		}
	} else {
		quint16 width = _image.width(), height = _image.height();
		const QBitArray &alphaBit = _alphaBits.first();

		data.append((char *)&sizeImgSection, 4);
		data.append((char *)&imgX, 2);
		data.append((char *)&imgY, 2);
		data.append((char *)&width, 2);
		data.append((char *)&height, 2);

		for (int y=0; y<height; ++y) {
			for (int x=0; x<width; ++x) {
				if (bpp == 2) {
					quint16 color = PsColor::toPsColor(_image.pixel(x, y));
					setPsColorAlphaBit(color, alphaBit.at(y * width + x));
					data.append((char *)&color, 2);
				} else {
					QRgb c = _image.pixel(x, y);
					quint32 color = ((qRed(c) & 0xFF) << 16) | ((qGreen(c) & 0xFF) << 8) | (qBlue(c) & 0xFF);
					data.append((char *)&color, 3);
				}
			}
		}
	}

	return true;
}

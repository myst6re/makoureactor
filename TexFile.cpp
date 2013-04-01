/****************************************************************************
 ** Deling Final Fantasy VIII Field Editor
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
#include "TexFile.h"
#include "Palette.h"

TexFile::TexFile(const QByteArray &data) :
    TextureFile()
{
    open(data);
}

TexFile::TexFile(const TextureFile &textureFile, const TexStruct &header,
		const QVector<quint8> &colorKeyArray) :
	TextureFile(textureFile), header(header), colorKeyArray(colorKeyArray)
{
}

bool TexFile::open(const QByteArray &data)
{
    const char *constData = data.constData();
	quint32 w, h, headerSize, paletteSectionSize, imageSectionSize, colorKeySectionSize;

	if((quint32)data.size() < sizeof(TexStruct)) {
		qWarning() << "tex size too short!";
		return false;
	}

	memcpy(&header, constData, sizeof(TexStruct));

	if(header.version == 1) {
		headerSize = sizeof(TexStruct) - 4;
	} else if(header.version == 2) {
		headerSize = sizeof(TexStruct);
	} else {
		qWarning() << "unknown tex version!";
		return false;
	}

	w = header.imageWidth;
	h = header.imageHeight;
	paletteSectionSize = header.nbPalettes > 0 ? header.paletteSize * 4 : 0;
	imageSectionSize = w * h * header.bytesPerPixel;
	colorKeySectionSize = header.hasColorKeyArray ? header.nbPalettes : 0;

	if((quint32)data.size() != headerSize + paletteSectionSize + imageSectionSize + colorKeySectionSize) {
		qWarning() << "tex invalid size!";
		return false;
	}

	quint32 i;

	if(header.nbPalettes > 0)
	{
		quint32 index, imageStart = headerSize + paletteSectionSize;

		for(quint32 palID=0 ; palID < header.nbPalettes ; ++palID) {
			quint32 paletteStart = headerSize+header.nbColorsPerPalette1*4*palID;

			_image = QImage(w, h, QImage::Format_Indexed8);
			QVector<QRgb> colors;

			for(i=0 ; i<header.nbColorsPerPalette1 ; ++i)
			{
				index = paletteStart + i*4;
				colors.append(qRgba(data.at(index+2), data.at(index+1), data.at(index), data.at(index+3)));
			}

			_colorTables.append(colors);
		}

		_image.setColorTable(_colorTables.first());

		for(i=0 ; i<imageSectionSize ; ++i)
		{
			_image.setPixel(i % w, i / w, (quint8)data.at(imageStart+i));
		}

		if(header.hasColorKeyArray) {
			quint32 colorKeyStart = imageStart + imageSectionSize;

			for(quint32 j=0 ; j<header.nbPalettes ; ++j) {
				colorKeyArray.append(data.at(colorKeyStart+j));
			}
		}
    }
    else
    {
		quint16 color;
		_image = QImage(w, h, QImage::Format_ARGB32);
		QRgb *pixels = (QRgb *)_image.bits();

		for(i=0 ; i<imageSectionSize ; i+=header.bytesPerPixel) {
			if(header.bytesPerPixel == 2) {
				memcpy(&color, &constData[headerSize+i], 2);
				pixels[i/2] = PsColor::fromPsColor(color);
			} else if(header.bytesPerPixel == 3) {
				pixels[i/3] = qRgb(constData[headerSize+i], constData[headerSize+i+1], constData[headerSize+i+2]);
			}
        }
	}

    return true;
}

bool TexFile::save(QByteArray &data)
{
	data.append((char *)&header, header.version==2 ? sizeof(TexStruct) : sizeof(TexStruct) - 4);

	if(isPaletted()) {
		quint32 palID;

		for(palID=0 ; palID < header.nbPalettes && palID < (quint32)_colorTables.size() ; ++palID) {
			const QVector<QRgb> &palette = _colorTables.at(palID);
			quint32 colorID;
			for(colorID=0 ; colorID < header.nbColorsPerPalette1 && colorID < (quint32)palette.size() ; ++colorID) {
				const QRgb &color = palette.at(colorID);
				data.append((char)qBlue(color));
				data.append((char)qGreen(color));
				data.append((char)qRed(color));
				data.append((char)qAlpha(color));
			}
			for( ; colorID < header.nbColorsPerPalette1 ; ++colorID) {
				const QRgb color = qRgba(0, 0, 0, 0);
				data.append((char *)&color, 4);
			}
		}

		for( ; palID < header.nbPalettes ; ++palID) {
			for(quint32 colorID=0 ; colorID < header.nbColorsPerPalette1 ; ++colorID) {
				const QRgb color = qRgba(0, 0, 0, 0);
				data.append((char *)&color, 4);
			}
		}

		for(int y=0 ; y<_image.height() ; ++y) {
			for(int x=0 ; x<_image.width() ; ++x) {
				data.append((char)_image.pixelIndex(x, y));
			}
		}

		data.append((char *)colorKeyArray.data(), colorKeyArray.size());
	} else {
		QRgb *pixels = (QRgb *)_image.bits();
		for(int i=0 ; i<_image.width()*_image.height() ; ++i) {
			quint16 color = PsColor::toPsColor(pixels[i]);
			data.append((char *)&color, 2);
		}
	}

	return true;
}

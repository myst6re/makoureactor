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
#pragma once

#include <QtCore>
#include <QImage>
#include "TextureFile.h"

struct TexStruct {
	// Header
	quint32 version; // 1=FF7 | 2=FF8
	quint32 unknown1; // 0
	quint32 hasColorKey;
	quint32 unknown2; // 0
	quint32 unknown3; // 7
	quint32 minBitsPerColor;
	quint32 maxBitsPerColor;
	quint32 minAlphaBits;
	quint32 maxAlphaBits;
	quint32 minBitsPerPixel;
	quint32 maxBitsPerPixel;
	quint32 unknown4; // 0
	quint32 nbPalettes;
	quint32 nbColorsPerPalette1;
	quint32 bitDepth;
	quint32 imageWidth;
	quint32 imageHeight;
	quint32 pitch; // 0
	quint32 unknown5; // 0
	quint32 hasPal;
	quint32 bitsPerIndex;
	quint32 indexedTo8bit;
	quint32 paletteSize;
	quint32 nbColorsPerPalette2;// may be 0 sometimes
	quint32 runtimeData1;
	quint32 bitsPerPixel;
	quint32 bytesPerPixel;
	// Pixel format
	quint32 nbRedBits1;
	quint32 nbGreenBits1;
	quint32 nbBlueBits1;
	quint32 nbAlphaBits1;
	quint32 redBitmask;
	quint32 greenBitmask;
	quint32 blueBitmask;
	quint32 alphaBitmask;
	quint32 redShift;
	quint32 greenShift;
	quint32 blueShift;
	quint32 alphaShift;
	quint32 nbRedBits2;// Always 8 - Unused
	quint32 nbGreenBits2;// Always 8 - Unused
	quint32 nbBlueBits2;// Always 8 - Unused
	quint32 nbAlphaBits2;// Always 8 - Unused
	quint32 redMax;
	quint32 greenMax;
	quint32 blueMax;
	quint32 alphaMax;
	// /Pixel format
	quint32 hasColorKeyArray;
	quint32 runtimeData2;
	quint32 referenceAlpha;
	quint32 runtimeData3;
	quint32 unknown6;
	quint32 paletteIndex;
	quint32 runtimeData4;
	quint32 runtimeData5;
	quint32 unknown7;
	quint32 unknown8;
	quint32 unknown9;
	quint32 unknown10;
	quint32 unknown11; // only on ff8!
};

class TexFile : public TextureFile
{
public:
	enum Version {
		None, FF7, FF8
	};

	TexFile() : TextureFile() {}
	explicit TexFile(const QByteArray &data);
	TexFile(const TextureFile &textureFile, const TexStruct &header,
			const QVector<quint8> &colorKeyArray=QVector<quint8>());
	virtual bool open(const QByteArray &data);
	virtual bool save(QByteArray &data);
	TexFile scaled(const QSize &size) const;
	void setVersion(Version version);
	void debug();
private:
	void updateHeader();
	TexStruct header;
	QVector<quint8> colorKeyArray;
};

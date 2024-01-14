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
#include "BackgroundFilePC.h"
#include "BackgroundTextures.h"
#include "BackgroundIO.h"
#include "Palette.h"
#include "PaletteIO.h"
#include <PsColor>
#include "FieldPC.h"
#include <algorithm>

BackgroundFilePC::BackgroundFilePC(FieldPC *field) :
	BackgroundFile(field)
{
}

BackgroundFilePC::BackgroundFilePC(const BackgroundFilePC &other) :
	BackgroundFile(other)
{
	setTextures(new BackgroundTexturesPC(*other.textures()));
	PalettesPC palettes;
	for (Palette *pal : other.palettes()) {
		palettes.append(new PalettePC(*pal));
	}
	setPalettes(palettes);
}

void BackgroundFilePC::initEmpty()
{
	clear();

	BackgroundTexturesPC *textures = new BackgroundTexturesPC();
	BackgroundTexturesPCInfos infos;
	infos.pos = 0;
	infos.isBigTile = 0; // 16x16 tiles
	infos.depth = 2; // No palettes
	// Black
	textures->setTex(0, QList<uint>(256 * 256, qRgb(0, 0, 0)).toList(), infos);
	setTextures(textures);

	quint16 tileID = 0;
	QList<Tile> tiles;

	for (qint16 y = 128; y >= -256; y -= 128) {
		for (qint16 dstY = 0; dstY < 128; dstY += 16) {
			for (qint16 dstX = -256; dstX < 256; dstX += 16) {
				Tile t = Tile();
				t.ID = 4095;
				// Data optimization: always the same source
				t.srcX = 0;
				t.srcY = 0;
				t.dstX = dstX;
				t.dstY = y + dstY;
				t.size = 16;
				t.depth = 2;
				t.tileID = tileID++;
				tiles.append(t);
			}
		}
	}

	setTiles(BackgroundTiles(tiles));

	BackgroundFile::initEmpty();
}

bool BackgroundFilePC::open()
{
	if (isOpen() || isModified()) {
		setOpen(true);
		return true;
	}

	return open(field()->sectionData(Field::Background),
	            field()->sectionData(Field::PalettePC));
}

bool BackgroundFilePC::open(const QByteArray &data, const QByteArray &palData)
{
	QBuffer buff, palBuff;

	buff.setData(data);
	palBuff.setData(palData);

	BackgroundIOPC io(&buff, &palBuff, nullptr);
	if (!io.read(*this)) {
		return false;
	}

	setOpen(true);

	return true;
}

QByteArray BackgroundFilePC::save() const
{
	QBuffer buff, palBuff;

	BackgroundIOPC io(&buff, &palBuff, field()->inf());
	if (!io.write(*this)) {
		return QByteArray();
	}

	return buff.data();
}

QByteArray BackgroundFilePC::savePal() const
{
	QBuffer buff, palBuff;

	PaletteIOPC io(&palBuff, &buff);
	if (!io.write(palettes())) {
		return QByteArray();
	}

	return palBuff.data();
}

BackgroundFilePS BackgroundFilePC::toPS(FieldPS *field) const
{
	PalettesPS palettesPS = static_cast<const PalettesPC *>(&palettes())->toPS();
	BackgroundTiles tilesPS;
	BackgroundTexturesPS texturesPS = textures()->toPS(tiles(), tilesPS, palettesPS);

	BackgroundFilePS filePS(field);
	filePS.setPalettes(palettesPS);
	filePS.setTextures(new BackgroundTexturesPS(texturesPS));
	filePS.setTiles(tilesPS);
	filePS.setOpen(true);

	return filePS;
}

bool BackgroundFilePC::repair()
{
	// PC field file contains PS tiles format
	// Altough it is unused by the game, we can use it to repair the PC format
	BackgroundTilesFile *psTiles = field()->tiles();
	QMap<quint16, Tile> &tiles = reinterpret_cast<QMap<quint16, Tile> &>(tilesRef());
	QMutableMapIterator<quint16, Tile> it(tiles);
	bool modified = false;

	if (psTiles && psTiles->isOpen()) {
		const QMap<qint32, Tile> &psTilesList = psTiles->tiles().sortedTiles();

		while (it.hasNext()) {
			it.next();
			Tile &tile = it.value();
			const Tile tilePs = psTilesList.value((tile.layerID << 16) | tile.tileID);

			if (tile.depth < 2 && tile.paletteID != tilePs.paletteID) {
				tile.paletteID = tilePs.paletteID;
				tile.typeTrans = tilePs.typeTrans;
				modified = true;
			}
		}
	} else { // By hand
		int paletteCount = palettes().size();
		QSet<quint8> usedPalettes = this->tiles().usedPalettes();
		QList<quint8> unusedPalettes;

		// List unused palettes
		for (int palID = 0; palID < paletteCount; ++palID) {
			if (!usedPalettes.contains(quint8(palID))) {
				unusedPalettes.append(quint8(palID));
			}
		}

		std::sort(unusedPalettes.begin(), unusedPalettes.end(), std::less<quint8>());

		QMap<quint8, quint8> texToPalette;

		while (it.hasNext()) {
			it.next();
			Tile &tile = it.value();

			if (tile.depth < 2 && tile.blending && tile.typeTrans != 2 && tile.paletteID >= paletteCount) {
				tile.typeTrans = 2; // Modification in place
				if (texToPalette.contains(tile.textureID)) {
					tile.paletteID = texToPalette.value(tile.textureID);
					modified = true;
				} else if (!unusedPalettes.isEmpty()) {
					tile.paletteID = unusedPalettes.first();
					unusedPalettes.removeFirst();
					texToPalette.insert(tile.textureID, tile.paletteID);
					modified = true;
				} else {
					qWarning() << "BackgroundFilePC::repair cannot detect palette ID to use";
				}
			}
		}
	}

	if (modified) {
		setModified(true);
		return true;
	}

	return false;
}

bool BackgroundFilePC::addTile(Tile &tile, uint colorOrIndex)
{
	UnusedSpaceInTexturePC unusedSpace = textures()->findFirstUnusedSpaceInTextures(tiles(), tile.depth, tile.size);
	if (!unusedSpace.valid) {
		qWarning() << "not valid";
		return false;
	}

	tile.textureID = unusedSpace.texID;
	tile.srcX = unusedSpace.x;
	tile.srcY = unusedSpace.y;

	if (unusedSpace.needsToCreateTex) {
		BackgroundTexturesPCInfos texInfos;
		texInfos.depth = tile.depth;
		texInfos.isBigTile = tile.size == 32;
		// Ensure black or first palette index
		textures()->setTex(unusedSpace.texID, QList<uint>(256 * 256, tile.depth == 2 ? 0xFF000000 : 0), texInfos);
	}

	// If colorOrIndex not specified, ensure black or first palette index
	textures()->setTile(tile, QList<uint>(tile.size * tile.size, tile.depth == 2 ? colorOrIndex : (colorOrIndex > 0xFF ? 0 : colorOrIndex)));

	return BackgroundFile::addTile(tile, colorOrIndex);
}

struct BackgroundTexturePCInfosAndColors
{
	BackgroundTexturePCInfosAndColors(quint32 pos, quint8 depth, quint8 isBigTile) :
	      indexesOrColors(256 * 256), pos(pos), srcX(0), srcY(0), depth(depth), isBigTile(isBigTile) {}
	BackgroundTexturesPCInfos infos() const {
		BackgroundTexturesPCInfos ret;
		ret.pos = pos;
		ret.depth = depth;
		ret.isBigTile = isBigTile;
		return ret;
	}
	QList<uint> indexesOrColors;
	quint32 pos;
	quint16 srcX, srcY;
	quint8 depth, isBigTile;
};

quint8 findInRange(QList<quint8> &unusedTextureIds, quint8 start, quint8 end)
{
	for (quint8 i = start; i < end; ++i) {
		if (unusedTextureIds.contains(i)) {
			return unusedTextureIds.takeAt(i);
		}
	}
	
	return 255;
}

bool BackgroundFilePC::compile()
{
	QMap<BackgroundTexturesPC::TextureGroups, QList<BackgroundTexturePCInfosAndColors> > texs;
	quint32 textureDataPos = 0;

	BackgroundTiles t = tiles().orderedTiles(true);
	for (Tile &tile: t) {
		QList<uint> indexOrRgbListTile = textures()->tile(tile);
		BackgroundTexturesPC::TextureGroups group = BackgroundTexturesPC::textureGroup(tile);

		if (texs[group].isEmpty() || texs[group].last().srcY >= 256) {
			quint8 depth = textures()->depth(tile);
			if (depth == 0) {
				depth = 1;
			}
			qDebug() << "BackgroundFilePC::compile" << "create Texture for group" << group << textureDataPos << depth << tile.size << tile.blending << tile.typeTrans;
			texs[group].append(BackgroundTexturePCInfosAndColors(textureDataPos, depth, tile.size == 32));
			textureDataPos += depth * 256 * 256;
		}

		BackgroundTexturePCInfosAndColors &texInfosAndColors = texs[group].last();

		tile.textureID = texs[group].size() - 1;
		tile.textureY = quint8(group);
		tile.srcX = texInfosAndColors.srcX;
		tile.srcY = texInfosAndColors.srcY;
		
		qDebug() << "BackgroundFilePC::compile" << "xy" << group << tile.srcX << tile.srcY << tile.size << tile.layerID;

		// Copy tile
		for (quint8 y = 0; y < tile.size; ++y) {
			for (quint8 x = 0; x < tile.size; ++x) {
				texInfosAndColors.indexesOrColors[(texInfosAndColors.srcY + y) * 256 + texInfosAndColors.srcX + x] = indexOrRgbListTile[y * tile.size + x];
			}
		}

		texInfosAndColors.srcX += tile.size;

		if (texInfosAndColors.srcX >= 256) {
			texInfosAndColors.srcX = 0;
			texInfosAndColors.srcY += tile.size;
		}
	}

	quint8 texID = 0;
	QList<quint8> unusedTextureIds;
	QMutableMapIterator<BackgroundTexturesPC::TextureGroups, QList<BackgroundTexturePCInfosAndColors> > it(texs);
	BackgroundTexturesPC *newTextures = new BackgroundTexturesPC();

	while (it.hasNext()) {
		it.next();
		BackgroundTexturesPC::TextureGroups group = it.key();
		quint8 groupId = quint8(group);
		QList<BackgroundTexturePCInfosAndColors> &list = it.value();
		
		// The game have its way to select the texture number, we do the same
		while (texID < groupId) {
			unusedTextureIds.append(texID);
			++texID;
		}
		
		for (int j = 0; j < list.size(); ++j) {
			BackgroundTexturePCInfosAndColors &textureInfAndCol = list[j];
			
			while (true) {
				const uint8_t originalTexId = texID;
				if (group < BackgroundTexturesPC::PalettedBlended) { // Paletted
					if (texID >= int(BackgroundTexturesPC::PalettedBlended)) {
						// Only 15 textures allowed for paletted non blended
						texID = findInRange(unusedTextureIds, 0, quint8(BackgroundTexturesPC::PalettedBlended));
					}
				} else if (group < BackgroundTexturesPC::PalettedBlendedAverage) { // Paletted blended
					if (texID < int(BackgroundTexturesPC::PalettedBlended) || texID >= int(BackgroundTexturesPC::PalettedBlendedAverage)) {
						// Only 9 textures allowed for paletted blended
						texID = findInRange(unusedTextureIds, quint8(BackgroundTexturesPC::PalettedBlended), quint8(BackgroundTexturesPC::PalettedBlendedAverage));
					}
				} else if (group < BackgroundTexturesPC::DirectColor) { // Paletted blended average
					// We can allow extra textures in this case, because the game does not check strictly the bounds
					if (texID < int(BackgroundTexturesPC::PalettedBlendedAverage) || (texID >= int(BackgroundTexturesPC::DirectColor) && texID < int(BackgroundTexturesPC::DirectColorBlended))) {
						// Only 2 textures allowed for paletted blended average...
						texID = findInRange(unusedTextureIds, quint8(BackgroundTexturesPC::PalettedBlendedAverage), quint8(BackgroundTexturesPC::DirectColor));
						if (texID >= BACKGROUND_TEXTURE_PC_MAX_COUNT) {
							// ...but we can add 9 textures from direct color blended!
							texID = findInRange(unusedTextureIds, quint8(BackgroundTexturesPC::DirectColorBlended), quint8(BACKGROUND_TEXTURE_PC_MAX_COUNT));
						}
					}
				} else if (group < BackgroundTexturesPC::DirectColorBlended) { // Direct color
					// We can allow extra textures in this case, because the game does not check strictly the bounds
					if ((texID >= int(BackgroundTexturesPC::PalettedBlended) && texID < int(BackgroundTexturesPC::DirectColor))
						|| texID >= int(BackgroundTexturesPC::DirectColorBlended)) {
						// Only 7 textures allowed for direct color...
						texID = findInRange(unusedTextureIds, quint8(BackgroundTexturesPC::DirectColor), quint8(BackgroundTexturesPC::DirectColorBlended));
						if (texID >= BACKGROUND_TEXTURE_PC_MAX_COUNT) {
							// ...but we can add 15 textures from paletted color!
							texID = findInRange(unusedTextureIds, 0, quint8(BackgroundTexturesPC::PalettedBlended));
						}
					}
				} else if (group < BackgroundTexturesPC::DirectColorBlendedAverage) { // Direct color blended
					if (texID < int(BackgroundTexturesPC::DirectColorBlended) || texID >= int(BackgroundTexturesPC::DirectColorBlendedAverage)) {
						// Only 7 textures allowed for direct color blended
						texID = findInRange(unusedTextureIds, quint8(BackgroundTexturesPC::DirectColorBlended), quint8(BackgroundTexturesPC::DirectColorBlendedAverage));
					}
				} else { // Direct color blended average
					if (texID < int(BackgroundTexturesPC::DirectColorBlendedAverage) || texID >= BACKGROUND_TEXTURE_PC_MAX_COUNT) {
						// Only 2 textures allowed for direct color blended average
						texID = findInRange(unusedTextureIds, quint8(BackgroundTexturesPC::DirectColorBlended), quint8(BackgroundTexturesPC::DirectColorBlendedAverage));
					}
				}
				
				if (texID >= BACKGROUND_TEXTURE_PC_MAX_COUNT) {
					QString errorMessage;
					if (group < BackgroundTexturesPC::PalettedBlended) { // Paletted
						errorMessage = QObject::tr("You cannot have more than 15 textures in paletted non-blended mode");
					} else if (group < BackgroundTexturesPC::DirectColor) { // Paletted blended
						errorMessage = QObject::tr("You cannot have more than 9 textures in paletted blended mode and 11 in paletted blended average mode");
					} else if (group < BackgroundTexturesPC::DirectColorBlended) { // Direct color
						errorMessage = QObject::tr("You cannot have more than 22 textures in direct color non-blended mode");
					} else { // Direct color blended
						errorMessage = QObject::tr("You cannot have more than 7 textures in direct color blended mode and 2 in direct color blended average mode");
					}
					_lastError = QObject::tr("Cannot find enough space to save textures of the background.\n%1.").arg(errorMessage);

					return false;
				}
				
				if (! newTextures->hasTex(texID)) {
					qDebug() << "BackgroundFilePC::compile" << "set tex" << texID << textureInfAndCol.infos().depth << textureInfAndCol.infos().isBigTile << textureInfAndCol.infos().pos;

					newTextures->setTex(texID, textureInfAndCol.indexesOrColors, textureInfAndCol.infos());
					textureInfAndCol.pos = texID;
					
					texID = originalTexId + 1;
					
					break;
				}
				
				texID = originalTexId + 1;
			}
		}
	}

	for (Tile &tile: t) {
		tile.textureID = texs[BackgroundTexturesPC::TextureGroups(tile.textureY)][tile.textureID].pos;
	}

	setTiles(t.orderedTiles(false));
	setTextures(newTextures);
	setModified(true);

	return true;
}

bool BackgroundFilePC::resize(const QSize &size)
{
	QRect rect = tiles().tiles(QList<quint8>() << 0 << 1).rect();
	Tile tile = Tile();
	tile.ID = 4095;
	tile.layerID = 0;
	tile.calcIDBig();
	tile.paletteID = 0;
	tile.size = 16;
	tile.depth = 2;
	uint indexOrColor = 0;

	BackgroundTiles layer0Tiles = tiles().tiles(0);
	for (const Tile &t : layer0Tiles) {
		if (t.depth == 1) {
			tile.depth = 1;
			indexOrColor = 1; // Prevent transparent pixel by using the first index

			break;
		}
	}

	if (rect.width() < size.width()) {
		quint32 diff = (size.width() - rect.width()) / tile.size;
		quint32 width = (diff / 2) * tile.size;
		QSize rectSize = QSize(width, rect.height());
		
		// Rect left
		createTiles(QRect(QPoint(-rect.x() - width, -rect.y()), rectSize), tile, indexOrColor);
		// Rect right
		createTiles(QRect(QPoint(-rect.x() + rect.width(), -rect.y()), rectSize), tile, indexOrColor);

		if (rect.height() < size.height()) {
			quint32 diff = (size.height() - rect.height()) / tile.size;
			quint32 height = (diff / 2) * tile.size;
			QSize rectSize = QSize(width, height);
			
			// Rect top
			createTiles(QRect(QPoint(-rect.x() - width, -rect.y() - height), rectSize), tile, indexOrColor);
			// Rect bottom
			createTiles(QRect(QPoint(-rect.x() - width, -rect.y() + rect.height()), rectSize), tile, indexOrColor);
		}

		return true;
	}
	if (rect.height() < size.height()) {
		quint32 diff = (size.height() - rect.height()) / tile.size;
		quint32 height = (diff / 2) * tile.size;
		QSize rectSize = QSize(rect.width(), height);
		
		// Rect top
		createTiles(QRect(QPoint(-rect.x(), -rect.y() - height), rectSize), tile, indexOrColor);
		// Rect bottom
		createTiles(QRect(QPoint(-rect.x(), -rect.y() + rect.height()), rectSize), tile, indexOrColor);

		return true;
	}

	return false;
}

bool BackgroundFilePC::addPalette(const char *data)
{
	if (_palettes.size() < 256) {
		_palettes.append(new PalettePC(data, false));
	
		setModified(true);
		
		return true;
	}

	return false;
}

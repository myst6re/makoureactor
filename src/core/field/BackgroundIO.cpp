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
#include "BackgroundIO.h"
#include "PaletteIO.h"
#include "BackgroundTilesIO.h"
#include "BackgroundTexturesIO.h"

BackgroundIO::BackgroundIO(QIODevice *device) :
	IO(device)
{
}

BackgroundIO::~BackgroundIO()
{
}

BackgroundIOPC::BackgroundIOPC(QIODevice *device, QIODevice *devicePal) :
	BackgroundIO(device), _devicePal(devicePal)
{
}

bool BackgroundIOPC::canReadPal() const
{
	if (_devicePal) {
		if (!_devicePal->isOpen()) {
			return _devicePal->open(QIODevice::ReadOnly);
		}
		return _devicePal->isReadable();
	}
	return false;
}

bool BackgroundIOPC::canWritePal() const
{
	if (_devicePal) {
		if (!_devicePal->isOpen()) {
			return _devicePal->open(QIODevice::WriteOnly);
		}
		return _devicePal->isWritable();
	}
	return false;
}

bool BackgroundIOPC::openPalettes(PalettesPC &palettes) const
{
	if (!_devicePal->reset() ||
			!device()->seek(12)) {
		return false;
	}

	PaletteIOPC io(_devicePal, device());
	if (!io.read(palettes)) {
		return false;
	}

	return true;
}

bool BackgroundIOPC::openTiles(BackgroundTiles &tiles) const
{
	if (!device()->reset()) {
		return false;
	}

	BackgroundTilesIOPC io(device());
	if (!io.read(tiles)) {
		return false;
	}

	return true;
}

bool BackgroundIOPC::openTextures(BackgroundTexturesPC &textures) const
{
	if (!device()->seek(device()->pos() + 7)) {
		return false;
	}

	BackgroundTexturesIOPC io(device());
	if (!io.read(&textures)) {
		return false;
	}

	return true;
}

bool BackgroundIOPC::read(BackgroundFile &background) const
{
	if (!canRead()
			|| !canReadPal()) {
		return false;
	}

	background.clear();

	PalettesPC palettes;
	BackgroundTiles tiles;
	BackgroundTexturesPC textures;

	if (!openPalettes(palettes)
			|| !openTiles(tiles)
			|| !openTextures(textures)) {
		qDeleteAll(palettes);
		return false;
	}

	background.setPalettes(palettes);
	background.setTiles(tiles);
	background.setTextures(new BackgroundTexturesPC(textures));

	return true;
}

bool BackgroundIOPC::write(const BackgroundFile &background) const
{
	if (!canWrite()
			|| !canWritePal()) {
		return false;
	}

	quint16 unknown1 = 0, depth = 1;
	quint8 unknown2 = 1;

	if (device()->write((char *)&unknown1, 2) != 2
			|| device()->write((char *)&depth, 2) != 2
			|| device()->write((char *)&unknown2, 1) != 1
			|| device()->write("PALETTE", 7) != 7) {
		return false;
	}

	PaletteIOPC paletteIO(devicePal(), device());
	if (!paletteIO.write(background.palettes())) {
		return false;
	}

	if (device()->write("BACK", 4) != 4) {
		return false;
	}

	BackgroundTilesIOPC backgroundTiles(device());

	if (!backgroundTiles.write(background.tiles())) {
		return false;
	}

	if (device()->write("TEXTURE", 7) != 7) {
		return false;
	}

	BackgroundTexturesIOPC backgroundTextures(device());

	if (!backgroundTextures.write(static_cast<BackgroundTexturesPC *>(background.textures()))) {
		return false;
	}

	if (device()->write("END", 3) != 3) {
		return false;
	}

	return true;
}

BackgroundIOPS::BackgroundIOPS(QIODevice *device, QIODevice *deviceTiles) :
	BackgroundIO(device), _deviceTiles(deviceTiles)
{
}

bool BackgroundIOPS::canReadTiles() const
{
	if (_deviceTiles) {
		if (!_deviceTiles->isOpen()) {
			return _deviceTiles->open(QIODevice::ReadOnly);
		}
		return _deviceTiles->isReadable();
	}
	return false;
}

bool BackgroundIOPS::canWriteTiles() const
{
	if (_deviceTiles) {
		if (!_deviceTiles->isOpen()) {
			return _deviceTiles->open(QIODevice::WriteOnly);
		}
		return _deviceTiles->isWritable();
	}
	return false;
}

bool BackgroundIOPS::openPalettes(PalettesPS &palettes) const
{
	if (!device()->reset()) {
		return false;
	}

	return PaletteIOPS(device()).read(palettes);
}

bool BackgroundIOPS::openTiles(BackgroundTiles &tiles) const
{
	if (!deviceTiles()->reset()) {
		return false;
	}

	return BackgroundTilesIOPS(deviceTiles()).read(tiles);
}

bool BackgroundIOPS::openTextures(BackgroundTexturesPS &textures) const
{
	if (!device()->reset()) {
		return false;
	}

	return BackgroundTexturesIOPS(device()).read(&textures);
}

bool BackgroundIOPS::savePalettes(const Palettes &palettes) const
{
	if (!device()->reset()) {
		return false;
	}

	return PaletteIOPS(device()).write(Palettes(palettes));
}

bool BackgroundIOPS::saveTiles(const BackgroundTiles &tiles) const
{
	if (!deviceTiles()->reset()) {
		return false;
	}

	return BackgroundTilesIOPS(deviceTiles()).write(tiles);
}

bool BackgroundIOPS::saveTextures(const BackgroundTexturesPS *textures) const
{
	// Assume device pos is after palette section
	return BackgroundTexturesIOPS(device()).write(textures);
}

bool BackgroundIOPS::read(BackgroundFile &background) const
{
	if (!canRead()
			|| !canReadTiles()) {
		return false;
	}

	background.clear();

	PalettesPS palettes;
	BackgroundTiles tiles;
	BackgroundTexturesPS *textures = new BackgroundTexturesPS();

	if (!openPalettes(palettes)
			|| !openTiles(tiles)
			|| !openTextures(*textures)) {
		qDeleteAll(palettes);
		delete textures;
		return false;
	}

	background.setPalettes(palettes);
	background.setTiles(tiles);
	background.setTextures(textures);

	return true;
}

bool BackgroundIOPS::write(const BackgroundFile &background) const
{
	if (!canWrite()
			|| !canWriteTiles()) {
		return false;
	}

	return saveTiles(background.tiles())
	        && savePalettes(background.palettes())
	        && saveTextures(static_cast<const BackgroundTexturesPS *>(background.textures()));
}

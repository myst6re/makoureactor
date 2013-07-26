/****************************************************************************
 ** Makou Reactor Final Fantasy VII Field Script Editor
 ** Copyright (C) 2009-2013 Arzel Jérôme <myst6re@gmail.com>
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
#ifndef PALETTEIO_H
#define PALETTEIO_H

#include <QtCore>
#include "Palette.h"

class PaletteIO
{
public:
	PaletteIO();
	explicit PaletteIO(QIODevice *device);
	virtual ~PaletteIO();

	void setDevice(QIODevice *device);
	QIODevice *device() const;

	bool canRead() const;
	bool canWrite() const;

	bool read(Palettes &palettes) const;
	bool write(const Palettes &palettes) const;
protected:
	virtual Palette *createPalette(const char *data) const=0;
	virtual bool readAfter(Palettes &palettes) const {
		Q_UNUSED(palettes)
		return true;
	}
	virtual bool writeAfter(const Palettes &palettes) const {
		Q_UNUSED(palettes)
		return true;
	}
private:
	QIODevice *_device;
};

class PaletteIOPC : public PaletteIO
{
public:
	explicit PaletteIOPC(QIODevice *device, QIODevice *deviceAlpha);

	void setDeviceAlpha(QIODevice *device);
	QIODevice *deviceAlpha() const;

	bool canReadAlpha() const;
	bool canWriteAlpha() const;
protected:
	inline Palette *createPalette(const char *data) const {
		return new PalettePC(data);
	}
	bool readAfter(Palettes &palettes) const;
	bool writeAfter(const Palettes &palettes) const;
private:
	QIODevice *_deviceAlpha;
};

class PaletteIOPS : public PaletteIO
{
public:
	explicit PaletteIOPS(QIODevice *device);
protected:
	inline Palette *createPalette(const char *data) const {
		return new PalettePS(data);
	}
};

#endif // PALETTEIO_H

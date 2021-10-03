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
#include "../IO.h"
#include "Palette.h"

class PaletteIO : public IO
{
public:
	explicit PaletteIO(QIODevice *device);
	virtual ~PaletteIO() override;

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
	inline Palette *createPalette(const char *data) const override {
		return new PalettePC(data);
	}
	bool readAfter(Palettes &palettes) const override;
	bool writeAfter(const Palettes &palettes) const override;
private:
	QIODevice *_deviceAlpha;
};

class PaletteIOPS : public PaletteIO
{
public:
	explicit PaletteIOPS(QIODevice *device);
protected:
	inline Palette *createPalette(const char *data) const override {
		return new PalettePS(data);
	}
};

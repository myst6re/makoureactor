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
#pragma once

#include "BackgroundFilePS.h"
#include "BackgroundTextures.h"

class FieldPC;
class FieldPS;

class BackgroundFilePC : public BackgroundFile
{
public:
	explicit BackgroundFilePC(FieldPC *field);
	BackgroundFilePC(const BackgroundFilePC &other);

	void initEmpty() override;
	bool open() override;
	bool open(const QByteArray &data, const QByteArray &palData);
	QByteArray save() const override;
	QByteArray savePal() const;
	virtual inline bool canSave() const override { return true; }
	inline virtual BackgroundTexturesPC *textures() const override {
		return static_cast<BackgroundTexturesPC *>(BackgroundFile::textures());
	}
	BackgroundFilePS toPS(FieldPS *field) const;
	bool repair() override;
	bool addTile(Tile &tile, uint colorOrIndex = 0) override;
	// Ensure consistency between tile list, textures, and palettes
	bool compile();
	bool resize(const QSize &size);
private:
	using BackgroundFile::open;
};

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

#include <QtCore>
#include "FieldPart.h"
#include "BackgroundTiles.h"

// This class is only used for repairing PC backgrounds
class BackgroundTilesFile : public FieldPart
{
public:
	explicit BackgroundTilesFile(Field *field);
	virtual ~BackgroundTilesFile() override;

	bool open() override;
	bool open(const QByteArray &data) override;
	QByteArray save() const override;
	void clear() override;

	inline const BackgroundTiles &tiles() const {
		return _tiles;
	}

private:
	BackgroundTiles _tiles;
};

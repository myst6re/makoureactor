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

#include "FieldArchive.h"
#include "TutFilePC.h"
#include "FieldArchiveIOPC.h"
#include "FieldPC.h"

class FieldArchivePC : public FieldArchive
{
public:
	FieldArchivePC();
	FieldArchivePC(const QString &path, FieldArchiveIO::Type type);
	virtual ~FieldArchivePC() override;
	inline bool isPC() const override { return true; }

	void clear() override;

	const FieldPC *field(quint32 id) const;
	FieldPC *field(quint32 id, bool open=true, bool dontOptimize=false);

	TutFilePC *tut(const QString &name);
	const QMap<QString, TutFilePC *> &tuts() const;
	void addTut(const QString &name);

	void setSaved() override;

	FieldArchiveIOPC *io() const override;

	void cleanModelLoader();
	void removeUnusedSections();
	void repairBackgroundsPC();
private:
	QMap<QString, TutFilePC *> _tuts;
};

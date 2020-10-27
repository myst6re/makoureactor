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
#ifndef DEF_SCRIPTLIST
#define DEF_SCRIPTLIST

#include <QtWidgets>
#include "core/field/GrpScript.h"

class ScriptList : public QListWidget
{
	Q_OBJECT
public:
	explicit ScriptList(QWidget *parent = 0);

	Script *currentScript();
	int selectedID();

	void fill(GrpScript *grpScript = 0);
	void localeRefresh();
	void scroll(int, bool focus=true);

private slots:
	void evidence(QListWidgetItem *current, QListWidgetItem *previous);

private:
	GrpScript *grpScript;
};

#endif

/****************************************************************************
 ** Makou Reactor Final Fantasy VII Field Script Editor
 ** Copyright (C) 2009-2020 Arzel Jérôme <myst6re@gmail.com>
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
#ifndef SCRIPTEDITORMOVIEPAGE_H
#define SCRIPTEDITORMOVIEPAGE_H

#include <QtWidgets>
#include "ScriptEditorView.h"

class ScriptEditorMoviePage : public ScriptEditorView
{
	Q_OBJECT
public:
	ScriptEditorMoviePage(Field *field, GrpScript *grpScript, Script *script, int opcodeID, QWidget *parent = 0);
	Opcode *opcode();
	void setOpcode(Opcode *opcode);
private slots:
	void setMovieListItemTexts(int discID);
private:
	void build();
	void buildDiscList();
	void buildMovieList(int discID);
	QComboBox *movieList, *discList;
};

#endif // SCRIPTEDITORMOVIEPAGE_H

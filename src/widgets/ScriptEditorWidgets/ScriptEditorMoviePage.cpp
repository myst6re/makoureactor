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
#include "ScriptEditorMoviePage.h"
#include "Data.h"

ScriptEditorMoviePage::ScriptEditorMoviePage(Field *field, GrpScript *grpScript, Script *script, int opcodeID, QWidget *parent) :
	ScriptEditorView(field, grpScript, script, opcodeID, parent)
{
}

void ScriptEditorMoviePage::build()
{
	discList = new QComboBox(this);
	buildDiscList();
	discList->setCurrentIndex(0);
	movieList = new QComboBox(this);
	buildMovieList(0);

	QGridLayout *layout = new QGridLayout(this);
	layout->addWidget(new QLabel(tr("Disc")), 0, 0);
	layout->addWidget(discList, 0, 1, 1, 3);
	layout->addWidget(new QLabel(tr("Video")), 1, 0);
	layout->addWidget(movieList, 1, 1, 1, 3);
	layout->setRowStretch(2, 1);
	layout->setContentsMargins(QMargins());

	connect(discList, SIGNAL(currentIndexChanged(int)), SLOT(setMovieListItemTexts(int)));
	connect(movieList, SIGNAL(currentIndexChanged(int)), SIGNAL(opcodeChanged()));
}

void ScriptEditorMoviePage::buildDiscList()
{
	for (int discID = 1; discID <= 3; ++discID) {
		discList->addItem(tr("Disc %1").arg(discID));
	}
}

void ScriptEditorMoviePage::buildMovieList(int discID)
{
	for (int i = 0; i < 256; ++i) {
		movieList->addItem(QString());
	}
	setMovieListItemTexts(discID);
}

void ScriptEditorMoviePage::setMovieListItemTexts(int discID)
{
	QStringList *movieNames;

	if (discID == 1) {
		movieNames = &Data::movie_names_cd2;
	} else if (discID == 2) {
		movieNames = &Data::movie_names_cd3;
	} else {
		movieNames = &Data::movie_names_cd1;
	}

	int nbItems = movieNames->size();
	for (int i = 0; i < nbItems; ++i) {
		movieList->setItemText(i, movieNames->at(i));
	}
	for (int i = nbItems; i < 256; ++i) {
		movieList->setItemText(i, QString::number(i));
	}
}

Opcode *ScriptEditorMoviePage::opcode()
{
	OpcodePMVIE *opcodePMVIE = (OpcodePMVIE *)opcodePtr();

	opcodePMVIE->movieID = movieList->currentIndex();

	return opcodePtr();
}

void ScriptEditorMoviePage::setOpcode(Opcode *opcode)
{
	ScriptEditorView::setOpcode(opcode);

	OpcodePMVIE *opcodePMVIE = (OpcodePMVIE *)opcode;

	movieList->setCurrentIndex(opcodePMVIE->movieID);
}

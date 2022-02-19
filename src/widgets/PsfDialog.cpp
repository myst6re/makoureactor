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
#include "PsfDialog.h"

PsfDialog::PsfDialog(const QString &title, QWidget *parent, Qt::WindowFlags f) :
      QDialog(parent, f)
{
	_psflib = new QLineEdit("Final Fantasy 7.psflib", this);
	_title = new QLineEdit(title, this);
	_artist = new QLineEdit("Nobuo Uematsu", this);
	_game = new QLineEdit("Final Fantasy 7", this);
	_year = new QLineEdit("1997", this);
	_genre = new QLineEdit(this);
	_comment = new QTextEdit(this);
	_copyright = new QLineEdit("1997 Square", this);
	_author = new QLineEdit(MAKOU_REACTOR_NAME, this);

	QDialogButtonBox *buttonBox = new QDialogButtonBox(
	        QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);

	QGridLayout *layout = new QGridLayout(this);
	layout->addWidget(new QLabel(tr("psflib")), 0, 0, 1, 1);
	layout->addWidget(_psflib, 0, 1, 1, 3);
	layout->addWidget(new QLabel(tr("Title")), 1, 0, 1, 1);
	layout->addWidget(_title, 1, 1, 1, 3);
	layout->addWidget(new QLabel(tr("Artist")), 2, 0, 1, 1);
	layout->addWidget(_artist, 2, 1, 1, 3);
	layout->addWidget(new QLabel(tr("Game")), 3, 0, 1, 1);
	layout->addWidget(_game, 3, 1, 1, 3);
	layout->addWidget(new QLabel(tr("Year")), 4, 0, 1, 1);
	layout->addWidget(_year, 4, 1, 1, 1);
	layout->addWidget(new QLabel(tr("Genre")), 4, 2, 1, 1);
	layout->addWidget(_genre, 4, 3, 1, 1);
	layout->addWidget(new QLabel(tr("Comment")), 5, 0, 4, 1);
	layout->addWidget(_comment, 5, 1, 4, 3);
	layout->addWidget(new QLabel(tr("Copyright")), 9, 0, 1, 1);
	layout->addWidget(_copyright, 9, 1, 1, 3);
	layout->addWidget(new QLabel(tr("Author")), 10, 0, 1, 1);
	layout->addWidget(_author, 10, 1, 1, 3);
	layout->addWidget(buttonBox, 11, 0, 1, 4);

	connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
	connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

PsfTags PsfDialog::tags() const
{
	return PsfTags(_psflib->text())
	    .setTitle(_title->text())
	    .setArtist(_artist->text())
	    .setGame(_game->text())
	    .setCustom("year", _year->text())
	    .setGenre(_genre->text())
	    .setComment(_comment->toPlainText())
	    .setCopyright(_copyright->text())
	    .setAuthor(_author->text());
}

void PsfDialog::setNoTitle(bool noTitle)
{
	_title->setDisabled(noTitle);
	if (noTitle) {
		_title->setText(tr("(auto)"));
	} else {
		_title->setText(QString());
	}
}

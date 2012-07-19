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
#ifndef TUTWIDGET_H
#define TUTWIDGET_H

#include <QtGui>
#include "Field.h"
#include "TutFile.h"

class TutWidget : public QDialog
{
	Q_OBJECT
public:
	TutWidget(Field *field, TutFile *tut, TutFile *tutPC, QWidget *parent=0);
private slots:
	void changeVersion(bool isPS);
	void showText(QListWidgetItem *item, QListWidgetItem *lastItem);
	void setTextChanged();
	void add();
	void del();
	void exportation();
	void importation();
private:
	void fillList();
	void saveText(QListWidgetItem *item);

	QRadioButton *versionPS;
	QListWidget *list;
	QPushButton *exportButton, *importButton;
	QPlainTextEdit *textEdit;
	Field *field;
	TutFile *tut, *tutPC, *currentTut, tutCpy, tutPCCpy;
	bool textChanged;
	QSet<quint8> usedTuts;
protected:
	void accept();
	void reject();
};

#endif // TUTWIDGET_H

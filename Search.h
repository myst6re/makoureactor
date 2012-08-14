/****************************************************************************
 ** Makou Reactor Final Fantasy VII Field Script Editor
 ** Copyright (C) 2012 Arzel Jérôme <myst6re@gmail.com>
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
#ifndef DEF_SEARCH
#define DEF_SEARCH

#include <QtGui>
#include "FieldArchive.h"

class Search : public QDialog
{
    Q_OBJECT

public:
	explicit Search(QWidget *parent=0);

	void setFieldArchive(FieldArchive *fieldArchive);
	void setOpcode(int opcode);
	void setScriptExec(int groupID, int scriptID);
	void updateRunSearch();

	int fileID;
	int grpScriptID;
	int scriptID;
	int opcodeID;

public slots:
	void changeFileID(int);
	void changeGrpScriptID(int);
	void changeScriptID(int);
	void changeOpcodeID(int);

private slots:
	void cancelSearching();
	void chercherSuivant();
	void chercherPrecedent();

private:
	void setSearchValues();

	FieldArchive *fieldArchive;
	QComboBox *champ;
	QComboBox *liste;
	QComboBox *opcode;
	QCheckBox *caseSens, *useRegexp;
	QSpinBox *champBank;
	QSpinBox *champAdress;
	QLineEdit *champValue;
	QGridLayout *grid;
	QPushButton *buttonSuiv;
	QPushButton *buttonPrec;
	QComboBox *executionGroup;
	QSpinBox *executionScript;

	int clef;
	QRegExp text;
	quint8 bank;
	quint8 adress;
	int value;
	quint8 e_script, e_group;
	bool cancel;

signals:
	void found(int, int, int, int);

protected:
	void showEvent(QShowEvent *event) {
		activateWindow();
		champ->setFocus();
		champ->lineEdit()->selectAll();
		QDialog::showEvent(event);
	}
};

#endif

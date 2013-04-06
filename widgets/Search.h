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
#include "core/field/FieldArchive.h"

class Search : public QDialog
{
    Q_OBJECT

public:
	explicit Search(QWidget *parent=0);

	void setFieldArchive(FieldArchive *fieldArchive);
	void setOpcode(int opcode);
	void setText(const QString &text);
	void setScriptExec(int groupID, int scriptID);
	void updateRunSearch();

	int fieldID;
	int grpScriptID;
	int scriptID;
	int opcodeID;
	int textID;
	int from;

public slots:
	void changeFieldID(int);
	void changeGrpScriptID(int);
	void changeScriptID(int);
	void changeOpcodeID(int);
	void changeTextID(int);
	void changeFrom(int);

private slots:
	void updateComboVarName();
	void updateChampAdress();
	void cancelSearching();
	void findNext();
	void findPrev();

private:
	QWidget *scriptPageWidget();
	QWidget *textPageWidget();
	void setSearchValues();

	QTabWidget *tabWidget;
	FieldArchive *fieldArchive;
	QComboBox *champ, *champ2;
	QComboBox *liste;
	QComboBox *opcode;
	QCheckBox *caseSens, *useRegexp, *caseSens2, *useRegexp2;
	QSpinBox *champBank;
	QSpinBox *champAdress;
	QLineEdit *champValue;
	QComboBox *comboVarName;
	QPushButton *buttonNext, *buttonPrev;
	QLabel *returnToBegin;
	QComboBox *executionGroup;
	QSpinBox *executionScript;
	QComboBox *mapJump;

	int clef;
	QRegExp text;
	quint8 bank;
	quint8 adress;
	int value;
	quint8 e_script, e_group;
	quint16 field;
	bool cancel;

signals:
	void found(int, int, int, int);
	void foundText(int, int, int, int);

protected:
	void showEvent(QShowEvent *event) {
		activateWindow();
		champ->setFocus();
		champ->lineEdit()->selectAll();
		QDialog::showEvent(event);
	}
};

#endif

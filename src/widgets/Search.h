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

#include <QtWidgets>
#include "core/field/FieldArchive.h"
#include "SearchAll.h"

class Window;

class Search : public QDialog
{
    Q_OBJECT

public:
	explicit Search(Window *mainWindow);

	void setFieldArchive(FieldArchive *fieldArchive);
	void setOpcode(int opcode, bool show = false);
	void setText(const QString &text);
	void setScriptExec(int groupID, int scriptID);
	void updateRunSearch();

private slots:
	void saveCurrentTab(int tab);
	void saveCurrentScriptTab(int tab);
	void updateComboVarName();
	void updateChampAdress();
	void updateSearchVarPlaceholder(int opIndex);
	void updateOpcode2(int opIndex);
	void cancelSearching();
	void findNext();
	void findPrev();
	void findAll();
	void replaceCurrent();
	void replaceAll();
	void updateCaseSensitivity(bool cs);
	void processEvents() const;

private:
	QWidget *scriptPageWidget();
	QWidget *textPageWidget();
	static QRegularExpression buildRegExp(const QString &lineEditText, bool caseSensitive, bool useRegexp);
	void setSearchValues();
	FieldArchive::SearchScope searchScope() const;
	QString lastMessage() const;
	QString firstMessage() const;
	inline Window *mainWindow() const {
		return reinterpret_cast<Window *>(parentWidget());
	}
	void setActionsEnabled(bool enable);
	bool findNextScript(FieldArchive::Sorting sorting, FieldArchive::SearchScope scope,
	                    int &mapID, int &grpScriptID,
	                    int &scriptID, int &opcodeID);
	bool findNextText(FieldArchive::Sorting sorting, FieldArchive::SearchScope scope,
	                  int &mapID, int &textID, qsizetype &from, qsizetype &size);
	bool findPrevScript(FieldArchive::Sorting sorting, FieldArchive::SearchScope scope,
	                    int &mapID, int &grpScriptID, int &scriptID, int &opcodeID);
	bool findPrevText(FieldArchive::Sorting sorting, FieldArchive::SearchScope scope,
	                  int &mapID, int &textID, qsizetype &index, qsizetype &from, qsizetype &size);

	SearchAll *searchAllDialog;
	QTabWidget *tabWidget;
	FieldArchive *fieldArchive;
	QComboBox *champ, *champ2, *replace2;
	QComboBox *liste;
	QComboBox *opcode, *opcode2;
	QCheckBox *caseSens, *useRegexp, *caseSens2, *useRegexp2;
	QSpinBox *champBank;
	QSpinBox *champAddress;
	QLineEdit *champValue;
	QComboBox *champOp;
	QComboBox *comboVarName;
	QPushButton *buttonNext, *buttonPrev, *buttonAll;
	QLabel *returnToBegin;
	QComboBox *executionGroup;
	QSpinBox *executionScript;
	QComboBox *mapJump;
	QRadioButton *currentFieldCheckBox,
	*currentGrpScriptCheckBox,
	*currentScriptCheckBox;
	QRadioButton *currentFieldCheckBox2,
	*currentTextCheckBox;
	QPushButton *replaceCurrentButton,
	*replaceAllButton;

	QRegularExpression text;
	int clef;
	quint16 address;
	quint16 map;
	Opcode::Operation op;
	int value;
	quint8 e_script, e_group;
	quint8 bank;
	bool cancel;
	bool atTheEnd, atTheBeginning;
	QShortcut *findNextShortcut = nullptr;
	QShortcut *findPreviousShortcut = nullptr;

signals:
	void found(int fieldID, int grpScriptID, int scriptID, int opcodeID);
	void foundText(int fieldID, int textID, qsizetype index, qsizetype size);
	void textReplaced(int fieldID, int textID);

protected:
	void showEvent(QShowEvent *event) override;
};

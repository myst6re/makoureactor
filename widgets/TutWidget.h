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

#include <QtWidgets>
#include "widgets/Listwidget.h"
#include "core/field/Field.h"
#include "core/field/TutFileStandard.h"
#include "core/field/TutFilePC.h"

class TutWidget : public QDialog
{
	Q_OBJECT
public:
	TutWidget(QWidget *parent=0);
	void fill(Field *field, TutFilePC *tutPC, bool reload=false);
	void clear();
signals:
	void modified();
private slots:
	void changeVersion(bool isPS);
	void showText(QListWidgetItem *item, QListWidgetItem *lastItem);
	void setTextChanged();
	void add();
	void del();
	void cutCurrent();
	void copyCurrent();
	void pasteOnCurrent();
	void exportation();
	void importation();
private:
	QWidget *buildTutPage();
	QWidget *buildSoundPage();
	void fillList();
	QListWidgetItem *createListItem(int id) const;
	void updateAkaoID(quint16 akaoID);
	void saveText(QListWidgetItem *item);
	int currentRow(QListWidgetItem *item=0) const;
	QList<int> selectedRows() const;
	void cut(const QList<int> &rows);
	void copy(const QList<int> &rows);
	void paste(int row);

	QStackedWidget *stackedWidget;
	QRadioButton *versionPS, *versionPC;
	QListWidget *list;
	QPushButton *exportButton, *importButton;
	QPlainTextEdit *textEdit;
	QLabel *akaoDesc;
	QComboBox *akaoIDList;
	Field *field;
	TutFile *currentTut;
	TutFileStandard *tut;
	TutFilePC *tutPC;
	QSet<quint8> usedTuts;
	QList<QByteArray> _copiedData;
	bool copied;
};

#endif // TUTWIDGET_H

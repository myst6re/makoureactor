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
#ifndef DEF_TEXTMANAGER
#define DEF_TEXTMANAGER

#include "Field.h"
#include "FF7Text.h"
#include "TextHighlighter.h"
#include "TextPreview.h"

class TextManager : public QWidget
{
	Q_OBJECT

public:
	explicit TextManager(QWidget *parent=0);
	void setField(Field *field);
	void updateText();
	void gotoText(int textID, int from, int size);
signals:
	void modified();
	void textIDChanged(int);
	void fromChanged(int);
private slots:
	void selectText(QListWidgetItem *, QListWidgetItem *previous=0);
	void showList();
	void setTextChanged();
	void addText();
	void delText();
	void insertTag(QAction *action);
	void prevTextPreviewPage();
	void nextTextPreviewPage();
	void changeTextPreviewPage();
	void prevTextPreviewWin();
	void nextTextPreviewWin();
	void changeTextPreviewWin();
	void changeRect(QRect rect);
	void changeXCoord(int);
	void changeYCoord(int);
	void emitFromChanged();
private:
	void updateWindowCoord();

	QCheckBox *dispUnusedText;
	QListWidget *liste1;
	QToolBar *toolBar, *toolBar2;
	QMenu *menu1, *menu2;
	QPlainTextEdit *textEdit;
	TextPreview *textPreview;
	QToolButton *prevPage, *prevWin;
	QToolButton *nextPage, *nextWin;
	QLabel *textPage, *textWin;
	QSpinBox *xCoord, *yCoord;
	QSpinBox *wSize, *hSize;

	Field* field;
	QSet<quint8> usedTexts;
protected:
	virtual void focusInEvent(QFocusEvent *e);
};

#endif

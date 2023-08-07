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

#include "core/field/Field.h"
#include "TextPreview.h"

class TextManager : public QDialog
{
	Q_OBJECT

public:
	explicit TextManager(QWidget *parent = nullptr);
	void setField(Field *field, bool reload=false);
	void clear();
	void updateText();
	void gotoText(int textID, qsizetype from, qsizetype size);
	QString selectedText() const;
	int currentTextId() const;
	int currentTextPosition() const;
	int currentAnchorPosition() const;
signals:
	void modified();
	void opcodeModified(int groupID, int scriptID, int opcodeID);
public slots:
	void updateFromScripts();
	void updateNames();
private slots:
	void selectText(QListWidgetItem *item, QListWidgetItem *previous = nullptr);
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
	void changePosition(const QPoint &point);
	void changeSize(const QSize &size);
	void changeXCoord(int);
	void changeYCoord(int);
	void changeWSize(int);
	void changeHSize(int);
	void resizeWindow();
	inline void alignHorizontally() {
		align(Qt::AlignHCenter);
	}
	inline void alignVertically() {
		align(Qt::AlignVCenter);
	}
private:
	void updateWindowCoord();
	QList<FF7Window> getWindows(int textID) const;
	void align(Qt::Alignment alignment);

	QCheckBox *dispUnusedText;
	QListWidget *liste1;
	QToolBar *toolBar, *toolBar2;
	QMenu *menu1, *menu2, *menuVars, *menuKeys;
	QPlainTextEdit *textEdit;
	TextPreview *textPreview;
	QToolButton *prevPage, *prevWin;
	QToolButton *nextPage, *nextWin;
	QLabel *textPage, *textWin;
	QLabel *xLabel, *yLabel, *wLabel, *hLabel;
	QSpinBox *xCoord, *yCoord;
	QSpinBox *wSize, *hSize;
	QPushButton *hAlign, *vAlign, *autoSize;

	Section1File *scriptsAndTexts;
	QSet<quint8> usedTexts;
	static QIcon winIcon, noWinIcon;
protected:
	virtual void focusInEvent(QFocusEvent *e) override;
};

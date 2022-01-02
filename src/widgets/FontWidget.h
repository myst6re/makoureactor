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
#include "FontGrid.h"
#include "FontLetter.h"
#include "FontPalette.h"
#include "core/FF7Font.h"

class FontWidget : public QWidget
{
	Q_OBJECT
public:
	explicit FontWidget(QWidget *parent = nullptr);
	void clear();
	void setFF7Font(FF7Font *ff7Font);
	void setWindowBinFile(WindowBinFile *windowBinFile);
	void setReadOnly(bool ro);
signals:
	void letterEdited();
public slots:
	void setColor(int i);
	void setTable(int i);
	void setLetter(int i);
	void editLetter(const QString &letter);
	void editWidth(int w);
	void editLeftPadding(int padding);
	void exportFont();
	void importFont();
	void reset();
	void resetLetter();
private slots:
	void setModified();
private:
	FontGrid *fontGrid;
	QPushButton *exportButton, *importButton;
	FontLetter *fontLetter;
	FontPalette *fontPalette;
	QComboBox *selectPal, *selectTable;
	QPushButton *fromImage1, *fromImage2;
	QLineEdit *textLetter;
	QSpinBox *widthLetter, *leftPaddingLetter;
	QPushButton *resetButton2;
	FF7Font *ff7Font;
protected:
	void focusInEvent(QFocusEvent *) override;

};

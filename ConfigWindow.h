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
#ifndef CONFIGWINDOW_H
#define CONFIGWINDOW_H

#include <QtGui>

class ConfigWindow : public QDialog
{
	Q_OBJECT
public:
	ConfigWindow(QWidget *parent=0);
private:
	void fillConfig();
	static void setWindowColorIcon(QAbstractButton *widget, QRgb color);
	void setWindowColors();
	QTreeWidget *listFF7;
	QCheckBox *kernelAuto, *charAuto;
	QLabel *kernelPath, *charPath;
	QPushButton *ff7ButtonMod, *ff7ButtonRem, *kernelButton, *charButton;
	QCheckBox *disableOGL;
	QPushButton *windowColor1, *windowColor2, *windowColor3, *windowColor4, *windowColorReset;
	QLabel *windowPreview;
	QCheckBox *optiText, *japEnc;
	QRgb windowColorTopLeft, windowColorTopRight, windowColorBottomLeft, windowColorBottomRight;
private slots:
	void changeFF7ListButtonsState();
	void modifyCustomFF7Path();
	void removeCustomFF7Path();
	void kernelAutoChange(bool);
	void charAutoChange(bool);
	void changeKernelPath();
	void changeCharPath();
	void changeColor();
	void resetColor();
protected:
	void accept();
};

#endif // CONFIGWINDOW_H

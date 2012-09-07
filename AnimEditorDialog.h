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
#ifndef ANIMEDITORDIALOG_H
#define ANIMEDITORDIALOG_H

#include <QtGui>
#include "FieldModel.h"

class AnimEditorDialog : public QDialog
{
    Q_OBJECT
public:
	explicit AnimEditorDialog(int animID, const QGLWidget *shareWidget=0, QWidget *parent=0);
	int chosenAnimation();
private slots:
	void changeModelAnimation();
private:
	FieldModel *fieldModel;
	QListWidget *aList;
};

#endif // ANIMEDITORDIALOG_H

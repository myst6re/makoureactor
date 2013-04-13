/****************************************************************************
 ** Makou Reactor Final Fantasy VII Field Script Editor
 ** Copyright (C) 2009-2012 Arzel J�r�me <myst6re@gmail.com>
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
#ifndef MODELMANAGERPS_H
#define MODELMANAGERPS_H

#include <QtGui>
#include "core/field/FieldPS.h"
#include "ModelManager.h"

class ModelManagerPS : public ModelManager
{
	Q_OBJECT
public:
	ModelManagerPS(const QGLWidget *shareWidget=0, QWidget *parent=0);
protected:
	void fill2();
	QList<QStringList> modelNames() const;
	QList<QStringList> animNames(int row) const;
	void showModelInfos2(int row);
	void showModel2(QTreeWidgetItem *item);
	FieldModelLoaderPS *modelLoader() const;
	FieldPS *field() const;
};

#endif // MODELMANAGERPS_H

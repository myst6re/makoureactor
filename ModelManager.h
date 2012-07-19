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
#ifndef MODELMANAGER_H
#define MODELMANAGER_H

#include <QtGui>
#include "Field.h"
#include "ColorDisplay.h"
#include "FieldModel.h"

class ModelManager : public QDialog
{
	Q_OBJECT
public:
	ModelManager(Field *field, const QGLWidget *shareWidget=0, QWidget *parent=0);
private:
	FieldModelLoader *fieldModelLoader;
	bool page_filled;
	QSpinBox *typeHRC;
	QTreeWidget *models;
	QFrame *model_frame;
	QLineEdit *model_name;
	QSpinBox *model_unknown;
	QSpinBox *model_typeHRC;
	QTreeWidget *model_anims;
	FieldModel *model_preview;
	ColorDisplay *model_colorDisplay;
	QToolBar *toolBar2;

	QList<quint16> field_model_unknown;
	QStringList field_model_nameChar;
	QStringList field_model_nameHRC;
	QList<quint16> field_model_typeHRC;
	QList<QStringList> field_model_anims;
	QList< QList<QRgb> > field_colors;
private slots:
	void show_HRC_infos(QTreeWidgetItem *, QTreeWidgetItem *);
	void modifyHRC(const QString &hrc);
	void modifyAnimation(const QString &a);
	void add_HRC();
	void del_HRC();
	void up_HRC();
	void down_HRC();
	void renameOK_HRC(QTreeWidgetItem *);
	void add_anim();
	void del_anim();
	void up_anim();
	void down_anim();
	void renameOK_anim(QTreeWidgetItem *);
	void show_model(QTreeWidgetItem *item);
	void save_page(QTreeWidgetItem *);
protected:
	void accept();
};

#endif // MODELMANAGER_H

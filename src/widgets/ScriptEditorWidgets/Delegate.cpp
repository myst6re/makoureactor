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
/****************************************************************************
 **
 ** Copyright (C) 2009 Nokia Corporation and/or its subsidiary(-ies).
 ** Contact: Nokia Corporation (qt-info@nokia.com)
 **
 ** This file is part of the examples of the Qt Toolkit.
 **
 ** $QT_BEGIN_LICENSE:LGPL$
 ** Commercial Usage
 ** Licensees holding valid Qt Commercial licenses may use this file in
 ** accordance with the Qt Commercial License Agreement provided with the
 ** Software or, alternatively, in accordance with the terms contained in
 ** a written agreement between you and Nokia.
 **
 ** GNU Lesser General Public License Usage
 ** Alternatively, this file may be used under the terms of the GNU Lesser
 ** General Public License version 2.1 as published by the Free Software
 ** Foundation and appearing in the file LICENSE.LGPL included in the
 ** packaging of this file.  Please review the following information to
 ** ensure the GNU Lesser General Public License version 2.1 requirements
 ** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
 **
 ** In addition, as a special exception, Nokia gives you certain
 ** additional rights. These rights are described in the Nokia Qt LGPL
 ** Exception version 1.0, included in the file LGPL_EXCEPTION.txt in this
 ** package.
 **
 ** GNU General Public License Usage
 ** Alternatively, this file may be used under the terms of the GNU
 ** General Public License version 3.0 as published by the Free Software
 ** Foundation and appearing in the file LICENSE.GPL included in the
 ** packaging of this file.  Please review the following information to
 ** ensure the GNU General Public License version 3.0 requirements will be
 ** met: http://www.gnu.org/copyleft/gpl.html.
 **
 ** If you are unsure which license is appropriate for your use, please
 ** contact the sales department at http://www.qtsoftware.com/contact.
 ** $QT_END_LICENSE$
 **
 ****************************************************************************/

/*
	 delegate.cpp
	 
	 A delegate that allows the user to change integer values from the model
	 using a spin box widget.
 */

#include "Delegate.h"
#include "Data.h"
#include "core/Config.h"
#include "../KeyEditorDialog.h"
#include "../AnimEditorDialog.h"
#include "core/field/Section1File.h"
#include "core/field/Field.h"
#include "core/field/FieldArchiveIO.h"
#include "core/field/FieldArchive.h"
#include "widgets/ScriptEditorWidgets/ScriptEditorGenericList.h"

SpinBoxDelegate::SpinBoxDelegate(QObject *parent) :
    QItemDelegate(parent), _scriptsAndTexts(nullptr)
{
}

QWidget *SpinBoxDelegate::createEditor(QWidget *parent,
                                       const QStyleOptionViewItem &option,
                                       const QModelIndex &index) const
{
	Q_UNUSED(option)
	
	int type = index.data(Qt::UserRole + 2).toInt();
	int value = index.data(Qt::EditRole).toInt();
	if (type == ScriptEditorGenericList::field_id
	        && !Data::maplist(_scriptsAndTexts->field()->isPC()).isEmpty()) {
		QComboBox *comboBox = new QComboBox(parent);
		comboBox->setEditable(true);
		comboBox->setInsertPolicy(QComboBox::NoInsert);
		comboBox->completer()->setCompletionMode(QCompleter::PopupCompletion);
		comboBox->completer()->setFilterMode(Qt::MatchContains);
		const QStringList *mapList = nullptr;
		if (!_scriptsAndTexts->field()->io()->fieldArchive()->mapList().mapNames().empty()) {
			mapList = &_scriptsAndTexts->field()->io()->fieldArchive()->mapList().mapNames();
		} else {
			mapList = &Data::maplist(_scriptsAndTexts->field()->isPC());
		}
		int mapId = 0;
		for (const QString &mapName: *mapList) {
			comboBox->addItem(QString("%1 - %2").arg(mapId++).arg(mapName));
		}
		return comboBox;
	} else if (type == ScriptEditorGenericList::group_id
	          && _scriptsAndTexts->grpScriptCount() > 0) {
		QComboBox *comboBox = new QComboBox(parent);
		comboBox->setEditable(true);
		comboBox->setInsertPolicy(QComboBox::NoInsert);
		comboBox->completer()->setCompletionMode(QCompleter::PopupCompletion);
		comboBox->completer()->setFilterMode(Qt::MatchContains);
		for (const GrpScript &grp : _scriptsAndTexts->grpScripts()) {
			comboBox->addItem(grp.name());
		}
		return comboBox;
	} else if (type == ScriptEditorGenericList::personnage_id) {
		QComboBox *comboBox = new QComboBox(parent);
		comboBox->setEditable(true);
		comboBox->setInsertPolicy(QComboBox::NoInsert);
		comboBox->completer()->setCompletionMode(QCompleter::PopupCompletion);
		comboBox->completer()->setFilterMode(Qt::MatchContains);
		comboBox->addItems(Data::char_names);
		int nbItems = comboBox->count();
		for (int i=nbItems; i<100; i++)
			comboBox->addItem(QString("%1").arg(i));
		for (int i=100; i<254; i++)
			comboBox->addItem(Data::char_names.last());
		
		comboBox->addItem(tr("(Empty)"));
		comboBox->addItem(tr("(Empty)"));
		return comboBox;
	} else if (type == ScriptEditorGenericList::text_id
	          && _scriptsAndTexts->textCount() > 0) {
		QComboBox *comboBox = new QComboBox(parent);
		comboBox->setEditable(true);
		comboBox->setInsertPolicy(QComboBox::NoInsert);
		comboBox->completer()->setCompletionMode(QCompleter::PopupCompletion);
		comboBox->completer()->setFilterMode(Qt::MatchContains);
		for (const FF7String &t : _scriptsAndTexts->texts()) {
			comboBox->addItem(comboBox->fontMetrics().elidedText(t.text().simplified(), Qt::ElideRight, 640));
		}
		return comboBox;
	} else if (type == ScriptEditorGenericList::item_id
	          && !Data::item_names.isEmpty()) {
		QComboBox *comboBox = new QComboBox(parent);
		comboBox->setEditable(true);
		comboBox->setInsertPolicy(QComboBox::NoInsert);
		comboBox->completer()->setCompletionMode(QCompleter::PopupCompletion);
		comboBox->completer()->setFilterMode(Qt::MatchContains);
		comboBox->addItems(Data::item_names);
		
		int nbItems = comboBox->count();
		for (int i=nbItems; i<128; ++i) {
			comboBox->addItem(QString::number(i), i);
		}
		
		comboBox->addItems(Data::weapon_names);
		
		nbItems = comboBox->count();
		for (int i=nbItems; i<256; ++i) {
			comboBox->addItem(QString::number(i), i);
		}
		
		comboBox->addItems(Data::armor_names);
		
		nbItems = comboBox->count();
		for (int i=nbItems; i<288; ++i) {
			comboBox->addItem(QString::number(i), i);
		}
		
		comboBox->addItems(Data::accessory_names);
		
		nbItems = comboBox->count();
		for (int i=nbItems; i<512; ++i) {
			comboBox->addItem(QString::number(i), i);
		}
		
		return comboBox;
	} else if (type == ScriptEditorGenericList::materia_id
	          && !Data::materia_names.isEmpty()) {
		QComboBox *comboBox = new QComboBox(parent);
		comboBox->setEditable(true);
		comboBox->setInsertPolicy(QComboBox::NoInsert);
		comboBox->completer()->setCompletionMode(QCompleter::PopupCompletion);
		comboBox->completer()->setFilterMode(Qt::MatchContains);
		comboBox->addItems(Data::materia_names);
		int nbItems = comboBox->count();
		for (int i=nbItems; i<256; ++i) {
			comboBox->addItem(QString::number(i), i);
		}
		return comboBox;
	} else if (type == ScriptEditorGenericList::animation_id
	          && Data::currentModelID!=-1 && Data::currentHrcNames
	          && Data::currentAnimNames) {
		AnimEditorDialog dialog(value, parent);
		QComboBox *editor = new QComboBox(parent);
		editor->addItems(Data::currentAnimNames->value(Data::currentModelID));
		if (dialog.exec() == QDialog::Accepted) {
			editor->setCurrentIndex(dialog.chosenAnimation());
		} else {
			editor->setCurrentIndex(value);
		}
		return editor;
	} else if (type == ScriptEditorGenericList::movie_id
	          && !Data::movie_names_cd1.isEmpty()) {
		QComboBox *comboBox = new QComboBox(parent);
		comboBox->setEditable(true);
		comboBox->setInsertPolicy(QComboBox::NoInsert);
		comboBox->completer()->setCompletionMode(QCompleter::PopupCompletion);
		comboBox->completer()->setFilterMode(Qt::MatchContains);
		comboBox->addItems(Data::movie_names_cd1);
		int nbItems = comboBox->count();
		for (int i = nbItems; i < 256; ++i) {
			comboBox->addItem(QString::number(i), i);
		}
		return comboBox;
	} else if (type == ScriptEditorGenericList::operateur) {
		QComboBox *comboBox = new QComboBox(parent);
		comboBox->setEditable(true);
		comboBox->setInsertPolicy(QComboBox::NoInsert);
		comboBox->completer()->setCompletionMode(QCompleter::PopupCompletion);
		comboBox->completer()->setFilterMode(Qt::MatchContains);
		for (int i=0; i<OPERATORS_SIZE; ++i) {
			comboBox->addItem(QString::fromUtf8(Opcode::operators[i]), i);
		}
		for (int i=OPERATORS_SIZE; i<256; ++i) {
			comboBox->addItem(QString::number(i), i);
		}
		return comboBox;
	} else if (type == ScriptEditorGenericList::akao) {
		QComboBox *comboBox = new QComboBox(parent);
		comboBox->setEditable(true);
		comboBox->setInsertPolicy(QComboBox::NoInsert);
		comboBox->completer()->setCompletionMode(QCompleter::PopupCompletion);
		comboBox->completer()->setFilterMode(Qt::MatchContains);
		QList<quint8> unknownItems;
		for (quint16 i = 0; i < 256; ++i) {
			bool ok;
			QString str = Opcode::akao(quint8(i), &ok);
			if (ok) {
				comboBox->addItem(str, i);
			} else {
				unknownItems.append(quint8(i));
			}
		}
		for (quint8 i : unknownItems) {
			comboBox->addItem(QString::number(i), i);
		}

		return comboBox;
	} else if (type == ScriptEditorGenericList::keys
	          && !Data::key_names.isEmpty()) {
		KeyEditorDialog dialog(quint16(value), parent);
		QSpinBox *editor = new QSpinBox(parent);
		editor->setMinimum(index.data(Qt::UserRole).toInt());
		editor->setMaximum(index.data(Qt::UserRole + 1).toInt());
		if (dialog.exec() == QDialog::Accepted) {
			editor->setValue(dialog.keys());
		} else {
			editor->setValue(value);
		}
		return editor;
	} else if (type == ScriptEditorGenericList::color) {
		QSpinBox *editor = new QSpinBox(parent);
		editor->setMinimum(index.data(Qt::UserRole).toInt());
		editor->setMaximum(index.data(Qt::UserRole+1).toInt());
		QColor color = qRgb(value & 0xFF, (value >> 8) & 0xFF, value >> 16);
		color = QColorDialog::getColor(color, parent, tr("Choose a new color"));
		if (color.isValid()) {
			QRgb rgb = color.rgb();
			editor->setValue((qBlue(rgb) << 16) | (qGreen(rgb) << 8) | qRed(rgb));
		} else {
			editor->setValue(value);
		}
		return editor;
	}
	QSpinBox *editor = new QSpinBox(parent);
	editor->setMinimum(index.data(Qt::UserRole).toInt());
	editor->setMaximum(index.data(Qt::UserRole+1).toInt());
	return editor;
}

void SpinBoxDelegate::setEditorData(QWidget *editor,
                                    const QModelIndex &index) const
{
	int value = index.model()->data(index, Qt::EditRole).toInt(),
	    type = index.data(Qt::UserRole + 2).toInt();

	if (type == ScriptEditorGenericList::akao) {
		QComboBox *comboBox = static_cast<QComboBox*>(editor);
		comboBox->setCurrentIndex(comboBox->findData(value));
	} else if ((type == ScriptEditorGenericList::field_id
	            && !Data::maplist(_scriptsAndTexts->field()->isPC()).isEmpty())
	           || (type == ScriptEditorGenericList::group_id
	               && _scriptsAndTexts->grpScriptCount() > 0)
	           || type == ScriptEditorGenericList::personnage_id
	           || (type == ScriptEditorGenericList::text_id
	               && _scriptsAndTexts->textCount() > 0)
	           || (type == ScriptEditorGenericList::item_id
	               && !Data::item_names.isEmpty())
	           || (type == ScriptEditorGenericList::materia_id
	               && !Data::materia_names.isEmpty())
	           || (type == ScriptEditorGenericList::movie_id
	               && !Data::movie_names_cd1.isEmpty())
	           || type == ScriptEditorGenericList::operateur) {
		QComboBox *comboBox = static_cast<QComboBox*>(editor);
		comboBox->setCurrentIndex(value);
	} else if ((type == ScriptEditorGenericList::keys
	            && !Data::key_names.isEmpty())
	           || type == ScriptEditorGenericList::color
	           || (type == ScriptEditorGenericList::animation_id
	               && Data::currentModelID!=-1 && Data::currentHrcNames
	               && Data::currentAnimNames)) {
		return;
	} else {
		QSpinBox *spinBox = static_cast<QSpinBox*>(editor);
		spinBox->setValue(value);
	}
}

void SpinBoxDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,
                                   const QModelIndex &index) const
{
	int value,
	    type = index.data(Qt::UserRole+2).toInt();

	if (type == ScriptEditorGenericList::akao) {
		QComboBox *comboBox = static_cast<QComboBox*>(editor);
		value = comboBox->itemData(comboBox->currentIndex()).toInt();
	} else if ((type == ScriptEditorGenericList::field_id
	            && !Data::maplist(_scriptsAndTexts->field()->isPC()).isEmpty())
	           || (type == ScriptEditorGenericList::group_id
	               && _scriptsAndTexts->grpScriptCount() > 0)
	           || type == ScriptEditorGenericList::personnage_id
	           || (type == ScriptEditorGenericList::text_id
	               && _scriptsAndTexts->textCount() > 0)
	           || (type == ScriptEditorGenericList::item_id
	               && !Data::item_names.isEmpty())
	           || (type == ScriptEditorGenericList::materia_id
	               && !Data::materia_names.isEmpty())
	           || (type == ScriptEditorGenericList::animation_id
	               && Data::currentModelID!=-1 && Data::currentHrcNames
	               && Data::currentAnimNames)
	           || (type == ScriptEditorGenericList::movie_id
	               && !Data::movie_names_cd1.isEmpty())
	           || type == ScriptEditorGenericList::operateur) {
		QComboBox *comboBox = static_cast<QComboBox*>(editor);
		value = comboBox->currentIndex();
	} else {
		QSpinBox *spinBox = static_cast<QSpinBox*>(editor);
		spinBox->interpretText();
		value = spinBox->value();
	}
	model->setData(index, value, Qt::EditRole);
}

void SpinBoxDelegate::updateEditorGeometry(QWidget *editor,
                                           const QStyleOptionViewItem &option, const QModelIndex &/* index */) const
{
	editor->setGeometry(option.rect);
}

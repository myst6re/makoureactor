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
#include "core/field/Opcode.h"

class FieldArchive;

class VarManager : public QWidget
{
	Q_OBJECT
public:
	explicit VarManager(FieldArchive *fieldArchive, QWidget *parent = nullptr);
	void setFieldArchive(FieldArchive *fieldArchive);
private slots:
	void scrollToList1(int);
	void scrollToList2(int);
	void fillForm();
	void changeBank(int);

	void renameVar();
	void save();
	void search();
	void processEvents() const;

private:
	QTreeWidgetItem *findList2Item(int);
	void fillList1();
	void fillList2();
	void findVar(const FF7Var &var, bool &foundR, bool &foundW, QSet<FF7Var::VarSize> &varSize);
	void colorizeItem(QTreeWidgetItem *item, const FF7Var &var);
	static quint8 itemAddress(QTreeWidgetItem *item);
	static int rowFromBank(quint8 bank);
	static QPair<quint8, quint8> banksFromRow(int row);

	QMap<quint16, QString> local_var_names;

	QSpinBox *bank;
	QSpinBox *address;
	QLineEdit *name;
	QPushButton *rename;

	QPushButton *searchButton, *ok;

	QListWidget *liste1;
	QTreeWidget *liste2;

	FieldArchive *fieldArchive;
	QList<FF7Var> allVars;
	QMap<FF7Var, QSet<QString> > _fieldNames;
};

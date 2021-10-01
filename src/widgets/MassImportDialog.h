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
#pragma once

#include <QtWidgets>
#include "core/field/FieldArchive.h"
#include "FormatSelectionWidget.h"

class MassImportDialog : public QDialog
{
	Q_OBJECT
public:
	enum ImportType {
		Fields, Akaos, Texts
	};

	explicit MassImportDialog(QWidget *parent = nullptr);
	void fill(const FieldArchive *fieldArchive, int currentMapId);
	QList<int> selectedFields() const;
	bool importModule(ImportType type) const;
	const QString &moduleFormat(ImportType type) const;
	QString directory() const;
private slots:
	void chooseImportDirectory();
	void selectCurrentField();
private:
	QListWidget *fieldList;
	QMap<ImportType, FormatSelectionWidget *> imports;
	QLineEdit *dirPath;
	QPushButton *changeDir;

	const FieldArchive *_fieldArchive;
	int _currentMapId;
protected:
	void accept();
};

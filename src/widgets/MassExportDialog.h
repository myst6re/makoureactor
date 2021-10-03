/****************************************************************************
 ** Makou Reactor Final Fantasy VII Field Script Editor
 ** Copyright (C) 2009-2021 Arzel Jérôme <myst6re@gmail.com>
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

class MassExportDialog : public QDialog
{
	Q_OBJECT
public:
	enum ExportType {
		Fields, Backgrounds, Akaos, Texts
	};

	explicit MassExportDialog(QWidget *parent = nullptr);
	void fill(const FieldArchive *fieldArchive, int currentMapId);
	QList<int> selectedFields() const;
	bool exportModule(ExportType type) const;
	const QString &moduleFormat(ExportType type) const;
	QString directory() const;
	bool overwrite() const;
private slots:
	void chooseExportDirectory();
	void selectCurrentField();
private:
	QListWidget *fieldList;
	QMap<ExportType, FormatSelectionWidget *> exports;
	QLineEdit *dirPath;
	QPushButton *changeDir;
	QCheckBox *overwriteIfExists;
	QPushButton *exportButton;

	const FieldArchive *_fieldArchive;
	int _currentMapId;
protected:
	void accept() override;
};

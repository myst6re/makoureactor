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
#ifndef MASSEXPORTDIALOG_H
#define MASSEXPORTDIALOG_H

#include <QtGui>
#include "core/field/FieldArchive.h"
#include "FormatSelectionWidget.h"

class MassExportDialog : public QDialog
{
	Q_OBJECT
public:
	explicit MassExportDialog(QWidget *parent = 0);
	void fill(FieldArchive *fieldArchive);
	QList<int> selectedFields() const;
	bool exportBackground() const;
	int exportBackgroundFormat() const;
	bool exportAkao() const;
	int exportAkaoFormat() const;
	bool exportText() const;
	int exportTextFormat() const;
	QString directory() const;
	bool overwrite() const;
private slots:
	void chooseExportDirectory();
private:
	QListWidget *fieldList;
	FormatSelectionWidget *bgExport, *akaoExport, *textExport;
	QPushButton *selectAll, *clearSelection, *selectCurrentField;
	QLineEdit *dirPath;
	QPushButton *changeDir;
	QCheckBox *overwriteIfExists;

	FieldArchive *_fieldArchive;
protected:
	void accept();
};

#endif // MASSEXPORTDIALOG_H

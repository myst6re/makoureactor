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
#include "FontManager.h"
#include "Data.h"
#include "core/Config.h"

FontManager::FontManager(QWidget *parent) :
      QDialog(parent, Qt::Dialog | Qt::WindowCloseButtonHint)
{
	setWindowTitle(tr("Font Manager"));
	setSizeGripEnabled(true);

	/* ListWidget *listWidget = new ListWidget(this);
	plusAction = listWidget->addAction(ListWidget::Add);
	connect(listWidget, &ListWidget::addTriggered, this, &FontManager::addFont);
	minusAction = listWidget->addAction(ListWidget::Remove);
	connect(listWidget, &ListWidget::removeTriggered, this, &FontManager::removeFont);
	toolbar1 = listWidget->toolBar();
	list1 = listWidget->listWidget(); */

	fontWidget = new FontWidget(this);
	fontWidget->setWindowBinFile(&Data::windowBin);

	QHBoxLayout *layout = new QHBoxLayout(this);
	//layout->addWidget(listWidget);
	layout->addWidget(fontWidget, 1);

	//fillList1();
	//setFont(list1->currentRow());

	//connect(list1, &QListWidget::currentRowChanged, this &FontManager::setFont);
}

void FontManager::fillList1()
{
	QString currentEncoding = Config::value("encoding", "00").toString();

	QStringList fontList = FF7Font::fontList();
	for (const QString &fontName : fontList) {
		QListWidgetItem *item;

		if (fontName == "00" || fontName == "01") {
			item = new QListWidgetItem(fontName == "00" ? tr("Latin") : tr("Japanese"));
			item->setData(Qt::UserRole, fontName);
			list1->addItem(item);
		} else {
			FF7Font *font = FF7Font::font(fontName);
			if (font) {
				item = new QListWidgetItem(font->name());
				item->setData(Qt::UserRole, fontName);
				list1->addItem(item);
			} else {
				continue;
			}
		}

		if (currentEncoding == fontName) {
			list1->setCurrentItem(item);
		}
	}
}

void FontManager::setFont(int id)
{
	QListWidgetItem *item = list1->item(id);

	if (!item)	return;

	QString fontName = item->data(Qt::UserRole).toString();

	FF7Font *font = FF7Font::font(fontName);
	if (font) {
		fontWidget->setFF7Font(font);
		minusAction->setEnabled(!font->isReadOnly());
	}
}

void FontManager::addFont()
{
	QString name, nameId;

	if (newNameDialog(name, nameId)) {
		if (FF7Font::addFont(nameId, "TODO", name)) {
			QListWidgetItem *item = new QListWidgetItem(name);
			item->setData(Qt::UserRole, nameId);
			list1->addItem(item);
		}
	}
}

bool FontManager::newNameDialog(QString &name, QString &nameId)
{
	QDialog dialog(this, Qt::Dialog | Qt::WindowCloseButtonHint);

	QLineEdit *nameEdit = new QLineEdit(&dialog);
	QLineEdit *fileNameEdit = new QLineEdit(&dialog);

	QPushButton *ok = new QPushButton(tr("OK"));
	ok->setDefault(true);

	QFormLayout *formLayout = new QFormLayout;
	formLayout->addRow(tr("&Displayed name:"), nameEdit);
	formLayout->addRow(tr("&File name:"), fileNameEdit);

	QVBoxLayout *layout = new QVBoxLayout(&dialog);
	layout->addLayout(formLayout, 1);
	layout->addWidget(ok, 0, Qt::AlignCenter);

	connect(ok, &QPushButton::clicked, &dialog, &QDialog::accept);

	if (dialog.exec() == QDialog::Accepted) {
		QString name1 = nameEdit->text();
		QString name2 = QDir::cleanPath(fileNameEdit->text());

		QStringList fontList = FF7Font::fontList();
		if (name1.isEmpty() || name2.isEmpty()
		    || fontList.contains(name1)
		    || QFile::exists(FF7Font::fontDirPath()+"/"+name2)) {
			QMessageBox::warning(this, tr("Choose another name"), tr("This name already exist or is invalid, "
			                                                         "please choose another."));
			return false;
		}

		name = name1;
		nameId = name2;
		return true;
	}
	return false;
}

void FontManager::removeFont()
{
	QList<QListWidgetItem *> items = list1->selectedItems();
	if (items.isEmpty())		return;

	if (QMessageBox::Yes != QMessageBox::question(this, tr("Remove font"),
	                                              tr("Do you want to remove the selected font?"),
	                                              QMessageBox::Yes | QMessageBox::Cancel)) {
		return;
	}

	QListWidgetItem *item = items.first();

	if (FF7Font::removeFont(item->data(Qt::UserRole).toString())) {
		delete item;
	}
}

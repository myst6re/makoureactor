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
#include "VarManager.h"
#include "FieldArchive.h"

VarManager::VarManager(FieldArchive *fieldArchive, QWidget *parent)
	: QWidget(parent, Qt::Tool)
{
	setWindowTitle(tr("Gestionnaire de variables"));
	QFont font;
	font.setPointSize(8);
	
	QGridLayout *globalLayout = new QGridLayout(this);
	
	QHBoxLayout *layout1 = new QHBoxLayout();
	
	bank = new QSpinBox(this);
	bank->setRange(1,15);
	adress = new QSpinBox(this);
	adress->setRange(0,255);
	name = new QLineEdit(this);
	name->setMaxLength(50);
	rename = new QPushButton(tr("Renommer"), this);
	rename->setEnabled(false);
	
	layout1->addWidget(bank);
	layout1->addWidget(adress);
	layout1->addWidget(name);
	layout1->addWidget(rename);
	
	QHBoxLayout *layout2 = new QHBoxLayout();
	
	liste1 = new QListWidget(this);
	liste1->setFixedWidth(40);
	liste1->setFont(font);
	
	liste2 = new QTreeWidget(this);
	liste2->setColumnCount(2);
	liste2->setHeaderLabels(QStringList() << tr("Adresse") << tr("Surnom"));
	liste2->setIndentation(0);
	liste2->setItemsExpandable(false);
	liste2->setSortingEnabled(true);
	liste2->setFont(font);
	
	layout2->addWidget(liste1);
	layout2->addWidget(liste2);
	
	QHBoxLayout *layout3 = new QHBoxLayout();
	
	searchButton = new QPushButton(tr("Adresses utilisées"), this);
	ok = new QPushButton(QApplication::style()->standardIcon(QStyle::SP_DialogSaveButton), tr("Enregistrer"), this);
	ok->setEnabled(false);
	
	layout3->addWidget(searchButton);
	layout3->addStretch();
	layout3->addWidget(ok);
	
	globalLayout->addLayout(layout1, 0, 0);
	globalLayout->addLayout(layout2, 1, 0);
	globalLayout->addLayout(layout3, 2, 0);

	setFieldArchive(fieldArchive);
	
	local_var_names = Var::get();
	
	fillList1();
	fillList2();
	changeBank(0);
	
	connect(bank, SIGNAL(valueChanged(int)), SLOT(scrollToList1(int)));
	connect(adress, SIGNAL(valueChanged(int)), SLOT(scrollToList2(int)));
	connect(liste1, SIGNAL(currentRowChanged(int)), SLOT(changeBank(int)));
	connect(liste2, SIGNAL(itemSelectionChanged()), SLOT(fillForm()));
	connect(name, SIGNAL(returnPressed()), SLOT(renameVar()));
	connect(rename, SIGNAL(released()), SLOT(renameVar()));
	connect(ok, SIGNAL(released()), SLOT(save()));
	connect(searchButton, SIGNAL(released()), SLOT(search()));
	
	adjustSize();
}

void VarManager::setFieldArchive(FieldArchive *fieldArchive)
{
	this->fieldArchive = fieldArchive;
	searchButton->setEnabled(fieldArchive != 0);
}

void VarManager::fillList1()
{
	liste1->clear();
	for(quint8 bankID=1 ; bankID<16 ; ++bankID)
	{
		liste1->addItem(QString("%1").arg(bankID, 2, 10, QChar('0')));
	}
	liste1->setCurrentRow(0);
}

void VarManager::fillList2()
{
	liste2->clear();
	for(quint16 adressID=0 ; adressID<256 ; ++adressID)
	{
		new QTreeWidgetItem(liste2, QStringList() << QString("%1").arg(adressID, 3) << "");
	}
	liste2->resizeColumnToContents(0);
	liste2->resizeColumnToContents(1);
	liste2->sortByColumn(0, Qt::AscendingOrder);
	liste2->setCurrentItem(liste2->topLevelItem(0));
	rename->setEnabled(true);
}

void VarManager::changeBank(int row)
{
	int b = row+1;
	bank->setValue(b);
	liste2->blockSignals(true);
	
	for(quint16 adressID=0 ; adressID<256 ; ++adressID)
	{
		QTreeWidgetItem *item = findList2Item(adressID);
		if(item==NULL)	continue;
		item->setText(1, local_var_names.value(adressID | ((row+1) << 8)));

		if(allVars.contains(FF7Var(b, adressID))) {
			item->setBackground(0, QColor(0xff,0xe5,0x99));
			item->setBackground(1, QColor(0xff,0xe5,0x99));
		} else {
			item->setBackground(0, QBrush());
			item->setBackground(1, QBrush());
		}
	}
	fillForm();
	
	liste2->blockSignals(false);
}

void VarManager::scrollToList1(int bankID)
{
	liste1->setCurrentRow(bankID-1);
	liste1->scrollToItem(liste1->item(bankID-1));
}

void VarManager::scrollToList2(int adressID)
{
	QTreeWidgetItem *item = findList2Item(adressID);
	if(item==NULL)	return;
	liste2->setCurrentItem(item);
	liste2->scrollToItem(item);
}

QTreeWidgetItem *VarManager::findList2Item(int adressID)
{
	QList<QTreeWidgetItem *> items = liste2->findItems(QString("%1").arg(adressID, 3), Qt::MatchExactly);
	if(items.isEmpty())	return NULL;
	return items.first();
}

void VarManager::fillForm()
{
	if(liste2->selectedItems().isEmpty())	return;
	QTreeWidgetItem *item = liste2->selectedItems().at(0);
	adress->setValue(item->text(0).toInt());
	name->setText(item->text(1));	
}

void VarManager::renameVar()
{
	if(liste2->selectedItems().isEmpty())	return;
	local_var_names.insert(adress->value() | (bank->value() << 8), name->text());
	liste2->selectedItems().first()->setText(1, name->text());
	ok->setEnabled(true);
}

void VarManager::save()
{
	int error = Var::save(local_var_names);
	switch(error)
	{
	case 1:QMessageBox::warning(this, tr("Erreur"), tr("Fichier vars.cfg inaccessible.\nÉchec de l'enregistrement."));
		return;
	}
	ok->setEnabled(false);
}

void VarManager::search()
{
	QMessageBox mess(QMessageBox::Information, tr("Recherche"), tr("Recherche des variables en cours, cela peut prendre une minute..."));
	mess.setWindowModality(Qt::ApplicationModal);
	mess.setWindowFlags(Qt::Dialog | Qt::WindowCloseButtonHint | Qt::MSWindowsFixedSizeDialogHint);
	mess.setStandardButtons(QMessageBox::NoButton);
	mess.show();
	allVars = fieldArchive->searchAllVars();
	int b = bank->value();
	
	for(int adress=0 ; adress<256 ; ++adress)
	{
		if(allVars.contains(FF7Var(b, adress))) {
			liste2->topLevelItem(adress)->setBackground(0, QColor(0xff,0xe5,0x99));
			liste2->topLevelItem(adress)->setBackground(1, QColor(0xff,0xe5,0x99));
		} else {
			liste2->topLevelItem(adress)->setBackground(0, QBrush());
			liste2->topLevelItem(adress)->setBackground(1, QBrush());
		}
	}
}

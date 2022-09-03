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
#include "VarManager.h"
#include "core/field/FieldArchive.h"
#include "core/Var.h"
#include "../Data.h"

VarManager::VarManager(FieldArchive *fieldArchive, QWidget *parent)
	: QWidget(parent, Qt::Tool)
{
	setWindowTitle(tr("Variable manager"));

	QHBoxLayout *layout1 = new QHBoxLayout();

	bank = new QSpinBox(this);
	bank->setRange(1,15);
	address = new QSpinBox(this);
	address->setRange(0,255);
	name = new QLineEdit(this);
	name->setMaxLength(255);
	rename = new QPushButton(QIcon::fromTheme(QStringLiteral("document-edit")), tr("Rename"), this);

	layout1->addWidget(bank);
	layout1->addWidget(address);
	layout1->addWidget(name);
	layout1->addWidget(rename);

	QHBoxLayout *layout2 = new QHBoxLayout();

	liste1 = new QListWidget(this);
	liste1->setFixedWidth(
	    liste1->fontMetrics().boundingRect(QString(" WW-WW ")).width()
	    + contentsMargins().left() + contentsMargins().right());

	liste2 = new QTreeWidget(this);
	liste2->setColumnCount(4);
	liste2->setHeaderLabels(QStringList() << tr("Address") << tr("Nickname") << tr("Operation") << tr("Size"));
	liste2->setIndentation(0);
	liste2->setItemsExpandable(false);
	liste2->setSortingEnabled(true);

	layout2->addWidget(liste1);
	layout2->addWidget(liste2);

	QLabel *helpIcon = new QLabel(this);
	helpIcon->setPixmap(QIcon::fromTheme(QStringLiteral("dialog-information")).pixmap(32));
	QLabel *help = new QLabel(tr("Var banks 08, 09 and 10 are temporary and do not appear in the game save. "
	                             "<br/>Other banks are stored in pair: for example 01-02 is in the same memory location, "
	                             "but the first is used to store 8-bit values and "
	                             "the second is used to store 16-bit values."),
	                          this);
	help->setTextFormat(Qt::RichText);
	help->setWordWrap(true);
	QHBoxLayout *helpLayout = new QHBoxLayout;
	helpLayout->addWidget(helpIcon);
	helpLayout->addWidget(help, 1);

	QHBoxLayout *layout3 = new QHBoxLayout();

	searchButton = new QPushButton(tr("Addresses Used"), this);
	ok = new QPushButton(QIcon::fromTheme(QStringLiteral("document-save")), tr("Save"), this);
	ok->setEnabled(false);

	layout3->addWidget(searchButton);
	layout3->addStretch();
	layout3->addWidget(ok);

	QVBoxLayout *globalLayout = new QVBoxLayout(this);
	globalLayout->addLayout(layout1);
	globalLayout->addLayout(layout2);
	globalLayout->addLayout(helpLayout);
	globalLayout->addLayout(layout3);

	setFieldArchive(fieldArchive);

	local_var_names = Var::get();

	fillList1();
	fillList2();
	liste1->setCurrentRow(0);
	liste2->setCurrentItem(liste2->topLevelItem(0));
	changeBank(0);
	fillForm();

	connect(bank, &QSpinBox::valueChanged, this, &VarManager::scrollToList1);
	connect(address, &QSpinBox::valueChanged, this, &VarManager::scrollToList2);
	connect(liste1, &QListWidget::currentRowChanged, this, &VarManager::changeBank);
	connect(liste2, &QTreeWidget::itemSelectionChanged, this, &VarManager::fillForm);
	connect(name, &QLineEdit::returnPressed, this, &VarManager::renameVar);
	connect(rename, &QPushButton::clicked, this, &VarManager::renameVar);
	connect(ok, &QPushButton::clicked, this, &VarManager::save);
	connect(searchButton, &QPushButton::clicked, this, &VarManager::search);

	adjustSize();
}

void VarManager::setFieldArchive(FieldArchive *fieldArchive)
{
	this->fieldArchive = fieldArchive;
	searchButton->setEnabled(fieldArchive != nullptr);
}

QPair<quint8, quint8> VarManager::banksFromRow(int row)
{
	switch (row) {
	case 0:
		return qMakePair(quint8(1), quint8(2));
	case 1:
		return qMakePair(quint8(3), quint8(4));
	case 2:
		return qMakePair(quint8(5), quint8(6));
	case 3:
		return qMakePair(quint8(11), quint8(12));
	case 4:
		return qMakePair(quint8(13), quint8(14));
	case 5:
		return qMakePair(quint8(15), quint8(7));
	}
	Q_ASSERT(false);
	return qMakePair(quint8(0), quint8(0));
}

int VarManager::rowFromBank(quint8 bank)
{
	switch (bank) {
	case 1:
	case 2:
		return 0;
	case 3:
	case 4:
		return 1;
	case 5:
	case 6:
		return 2;
	case 11:
	case 12:
		return 3;
	case 13:
	case 14:
		return 4;
	case 15:
	case 7:
		return 5;
	}
	Q_ASSERT(false);
	return -1;
}

void VarManager::fillList1()
{
	for (quint8 row=0; row<6; ++row) {
		QPair<quint8, quint8> pair = banksFromRow(row);
		liste1->addItem(QString("%1-%2")
						.arg(pair.first, 2, 10, QChar('0'))
						.arg(pair.second, 2, 10, QChar('0')));
	}
}

void VarManager::fillList2()
{
	for (quint16 addressID=0; addressID<256; ++addressID) {
		new QTreeWidgetItem(liste2, QStringList() << QString("%1").arg(addressID, 3));
	}

	liste2->resizeColumnToContents(0);
	liste2->resizeColumnToContents(1);
	liste2->sortByColumn(0, Qt::AscendingOrder);
}

void VarManager::changeBank(int row)
{
	QPair<quint8, quint8> banks = banksFromRow(row);
	quint8 b = banks.first,
			b2 = banks.second;
	bank->setValue(b);
	liste2->blockSignals(true);

	QTreeWidgetItemIterator it(liste2);
	while (*it) {
		QTreeWidgetItem *item = *it;
		quint16 addressID = itemAddress(item);
		QString varName1 = local_var_names.value(quint16(addressID | (quint16(b) << 8))),
		        varName2 = local_var_names.value(quint16(addressID | (quint16(b2) << 8))),
				varName;

		if (varName1.isEmpty()) {
			varName = varName2;
		} else if (varName2.isEmpty()) {
			varName = varName1;
		} else {
			varName = QString("%1, %2").arg(varName1, varName2);
		}

		item->setText(1, varName);
		colorizeItem(item, FF7Var(qint8(b), quint8(addressID), FF7Var::VarSize()));
		++it;
	}
	fillForm();

	liste2->blockSignals(false);
}

quint8 VarManager::itemAddress(QTreeWidgetItem *item)
{
	return quint8(item->text(0).toInt());
}

void VarManager::scrollToList1(int bankID)
{
	int row = rowFromBank(quint8(bankID));
	liste1->setCurrentRow(row);
	liste1->scrollToItem(liste1->item(row));
}

void VarManager::scrollToList2(int addressID)
{
	QTreeWidgetItem *item = findList2Item(addressID);
	if (item) {
		liste2->setCurrentItem(item);
		liste2->scrollToItem(item);
	}
}

QTreeWidgetItem *VarManager::findList2Item(int addressID)
{
	QList<QTreeWidgetItem *> items = liste2->findItems(QString("%1").arg(addressID, 3), Qt::MatchExactly);
	if (items.isEmpty())	return nullptr;
	return items.first();
}

void VarManager::fillForm()
{
	if (liste2->selectedItems().isEmpty())	return;
	QTreeWidgetItem *item = liste2->selectedItems().first();
	address->setValue(itemAddress(item));
	name->setText(item->text(1));
}

void VarManager::renameVar()
{
	if (liste2->selectedItems().isEmpty())	return;
	local_var_names.insert(quint16(address->value() | (bank->value() << 8)), name->text());
	liste2->selectedItems().first()->setText(1, name->text());
	ok->setEnabled(true);
}

void VarManager::save()
{
	if (!Var::save(local_var_names)) {
		QMessageBox::warning(this, tr("Error"), tr("Save Failed"));
	} else {
		ok->setEnabled(false);
	}
}

void VarManager::search()
{
	QMessageBox mess(QMessageBox::Information, tr("Searching"), tr("Searching, it may take a minute..."));
	mess.setWindowModality(Qt::ApplicationModal);
	mess.setWindowFlags(Qt::Dialog | Qt::WindowCloseButtonHint | Qt::MSWindowsFixedSizeDialogHint);
	mess.setStandardButtons(QMessageBox::NoButton);
	mess.show();
	QTimer t(this);
	connect(&t, &QTimer::timeout, this, &VarManager::processEvents);
	t.start(700);
	allVars = fieldArchive->searchAllVars(_fieldNames);
	quint8 b = quint8(bank->value());

	for (quint16 address=0; address<256; ++address) {
		colorizeItem(liste2->topLevelItem(address), FF7Var(qint8(b), quint8(address), FF7Var::VarSize()));
	}

	t.stop();
}

void VarManager::processEvents() const
{
	QCoreApplication::processEvents();
}

void VarManager::findVar(const FF7Var &var, bool &foundR, bool &foundW, QSet<FF7Var::VarSize> &varSize)
{
	qsizetype index = -1;

	forever {
		index = allVars.indexOf(var, index + 1);
		if (index < 0) {
			break;
		}
		FF7Var foundVar = allVars.at(index);
		varSize.insert(foundVar.size);
		if (foundVar.flags.testFlag(FF7Var::Writable)) {
			foundW = true;
		} else {
			foundR = true;
		}
	}
}

void VarManager::colorizeItem(QTreeWidgetItem *item, const FF7Var &var)
{
	QPair<quint8, quint8> banks = banksFromRow(rowFromBank(var.bank));
	bool foundR = false, foundW = false;
	QSet<FF7Var::VarSize> varSize;

	findVar(FF7Var(qint8(banks.first),  var.address, FF7Var::VarSize()), foundR, foundW, varSize);
	findVar(FF7Var(qint8(banks.second), var.address, FF7Var::VarSize()), foundR, foundW, varSize);

	QString rwText;
	QStringList sizeText;

	if (foundR || foundW) {
		QColor color = Data::color(Data::ColorEvidence);
		item->setBackground(0, color);
		item->setBackground(1, color);
		item->setBackground(2, color);
		item->setBackground(3, color);
		if (foundR && foundW) {
			rwText = tr("rw");
		} else if (foundR) {
			rwText = tr("r");
		} else {
			rwText = tr("w");
		}
		if (varSize.contains(FF7Var::Bit)) {
			sizeText.append(tr("bitfield"));
		}
		if (varSize.contains(FF7Var::Byte)) {
			sizeText.append(tr("1 Byte"));
		}
		if (varSize.contains(FF7Var::Word)) {
			sizeText.append(tr("2 Bytes"));
		}
		if (varSize.contains(FF7Var::SignedWord)) {
			sizeText.append(tr("2 Signed Bytes"));
		}
	} else {
		item->setBackground(0, palette().base().color());
		item->setBackground(1, palette().base().color());
		item->setBackground(2, palette().base().color());
		item->setBackground(3, palette().base().color());
		item->setForeground(0, palette().text().color());
		item->setForeground(1, palette().text().color());
		item->setForeground(2, palette().text().color());
		item->setForeground(3, palette().text().color());
	}
	item->setText(2, rwText);
	item->setText(3, sizeText.join(", "));
	//item->setText(4, QStringList(_fieldNames.value(var).toList()).join(", "));
}

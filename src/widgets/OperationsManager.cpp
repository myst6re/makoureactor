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
#include "OperationsManager.h"
#include "core/field/FieldArchive.h"

OperationsManager::OperationsManager(bool isPC, QWidget *parent) :
	QDialog(parent, Qt::Dialog | Qt::WindowCloseButtonHint)
{
	setWindowTitle(tr("Batch processing"));

	addOperation(CleanUnusedTexts, tr("Clean all unused texts"));
	addOperation(AutosizeTextWindows, tr("Autosize all text windows in the game"));
	addOperation(RemoveTexts, tr("Disable all texts in the game"));
	addOperation(RemoveBattles, tr("Disable all battles in the game"));
	if (isPC) {
		addOperation(CleanModelLoaderPC, tr("Clean unused data in field model loaders"));
		addOperation(RemoveUnusedSectionPC, tr("Clean unused data for field backgrounds"));
		addOperation(RepairBackgroundsPC, tr("Repair broken backgrounds (lastmap, fr_e)"));
	}

	QDialogButtonBox *buttonBox = new QDialogButtonBox;
	applyButton = buttonBox->addButton(tr("Apply"), QDialogButtonBox::AcceptRole);
	buttonBox->addButton(QDialogButtonBox::Cancel);

	QVBoxLayout *layout = new QVBoxLayout(this);
	for (QCheckBox *operation : qAsConst(_operations)) {
		layout->addWidget(operation);
	}
	layout->addStretch();
	layout->addWidget(buttonBox);

	for (QCheckBox *checkBox : qAsConst(_operations)) {
		connect(checkBox, &QCheckBox::toggled, this, &OperationsManager::updateApplyButton);
	}

	connect(buttonBox, &QDialogButtonBox::accepted, this, &OperationsManager::accept);
	connect(buttonBox, &QDialogButtonBox::rejected, this, &OperationsManager::reject);

	updateApplyButton();
}

void OperationsManager::addOperation(Operation op, const QString &description)
{
	_operations.insert(op, new QCheckBox(description, this));
}

OperationsManager::Operations OperationsManager::selectedOperations() const
{
	QMapIterator<Operation, QCheckBox *> it(_operations);
	Operations ret = Operations();

	while (it.hasNext()) {
		it.next();

		if (it.value()->isChecked()) {
			ret |= it.key();
		}
	}

	return ret;
}

void OperationsManager::updateApplyButton()
{
	bool enabled = false;

	for (QCheckBox *checkBox : qAsConst(_operations)) {
		if (checkBox->isChecked()) {
			enabled = true;
			break;
		}
	}

	applyButton->setEnabled(enabled);
}

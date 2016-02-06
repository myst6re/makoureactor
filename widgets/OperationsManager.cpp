/****************************************************************************
 ** Makou Reactor Final Fantasy VII Field Script Editor
 ** Copyright (C) 2009-2013 Arzel Jérôme <myst6re@gmail.com>
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

OperationsManager::OperationsManager(bool isPC, QWidget *parent) :
	QDialog(parent, Qt::Dialog | Qt::WindowCloseButtonHint)
{
	setWindowTitle(tr("Opérations diverses"));

	addOperation(CleanUnusedTexts, tr("Effacer tous les textes inutilisés"));
	addOperation(RemoveTexts, tr("Désactiver tous les textes du jeu"));
	addOperation(RemoveBattles, tr("Désactiver tous combats du jeu"));
	if(isPC) {
		addOperation(CleanModelLoaderPC, tr("Supprimer les données inutiles des listes des modèles 3D"));
		addOperation(RemoveUnusedSectionPC, tr("Supprimer les données inutilisées pour les décors"));
	}

	QDialogButtonBox *buttonBox = new QDialogButtonBox;
	applyButton = buttonBox->addButton(tr("Appliquer"), QDialogButtonBox::AcceptRole);
	buttonBox->addButton(QDialogButtonBox::Cancel);

	QVBoxLayout *layout = new QVBoxLayout(this);
	foreach(QCheckBox *operation, _operations) {
		layout->addWidget(operation);
	}
	layout->addStretch();
	layout->addWidget(buttonBox);

	foreach(QCheckBox *checkBox, _operations) {
		connect(checkBox, SIGNAL(toggled(bool)), SLOT(updateApplyButton()));
	}

	connect(buttonBox, SIGNAL(accepted()), SLOT(accept()));
	connect(buttonBox, SIGNAL(rejected()), SLOT(reject()));

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

	while(it.hasNext()) {
		it.next();

		if(it.value()->isChecked()) {
			ret |= it.key();
		}
	}

	return ret;
}

void OperationsManager::updateApplyButton()
{
	bool enabled = false;

	foreach(QCheckBox *checkBox, _operations) {
		if(checkBox->isChecked()) {
			enabled = true;
			break;
		}
	}

	applyButton->setEnabled(enabled);
}

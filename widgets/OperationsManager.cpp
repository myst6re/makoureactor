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

	QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Apply | QDialogButtonBox::Cancel);

	QVBoxLayout *layout = new QVBoxLayout(this);
	foreach(QCheckBox *operation, _operations) {
		layout->addWidget(operation);
	}
	layout->addStretch();
	layout->addWidget(buttonBox);

	connect(buttonBox->button(QDialogButtonBox::Apply), SIGNAL(clicked()), SLOT(accept()));
	connect(buttonBox, SIGNAL(rejected()), SLOT(reject()));
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

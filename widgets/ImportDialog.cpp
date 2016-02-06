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
#include "ImportDialog.h"

ImportDialog::ImportDialog(bool sourceSameTypeAsTarget, bool isDat, const QString &path, QWidget *parent) :
	QDialog(parent, Qt::Dialog | Qt::WindowCloseButtonHint)
{
	setWindowTitle(tr("Importer"));

	QGroupBox *group = new QGroupBox(tr("Importer"));
	scripts = new QCheckBox(tr("Scripts/Textes"));
	akaos = new QCheckBox(tr("Musiques/Tutoriels"));
	ca = new QCheckBox(tr("Caméra"));
	id = new QCheckBox(tr("Walkmesh"));
	encounter = new QCheckBox(tr("Combats aléatoires"));
	inf = new QCheckBox(tr("Déclencheurs/Liens entre les écrans"));
	model = new QCheckBox(tr("Liste des modèles 3D"));
	mim = new QCheckBox(tr("Background"));

	model->setEnabled(sourceSameTypeAsTarget);
	mim->setEnabled((isDat && !sourceSameTypeAsTarget) ||
					(!isDat && sourceSameTypeAsTarget));

	scripts->setChecked(scripts->isEnabled());
	akaos->setChecked(akaos->isEnabled());
	ca->setChecked(ca->isEnabled());
	id->setChecked(id->isEnabled());
	encounter->setChecked(encounter->isEnabled());
	inf->setChecked(inf->isEnabled());
	model->setChecked(model->isEnabled());
	mim->setChecked(mim->isEnabled());

	if(isDat) {
		model->setVisible(false);
	}

	pathEdit = new QLineEdit;
	QPushButton *changePathButton = new QPushButton(tr("Changer"));

	pathWidget = new QWidget;
	QHBoxLayout *layoutPath = new QHBoxLayout(pathWidget);
	layoutPath->addWidget(new QLabel(tr("Fichier MIM :")));
	layoutPath->addWidget(pathEdit);
	layoutPath->addWidget(changePathButton);
	layoutPath->setContentsMargins(QMargins());

	if(isDat) {
		pathWidget->setEnabled(mim->isChecked());
		int index = path.lastIndexOf(".");
		pathEdit->setText(index > -1 ? (path.left(index + 1) + "MIM")
									 : path);
	} else {
		pathWidget->hide();
	}

	QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal);

	QVBoxLayout *layout = new QVBoxLayout(group);
	layout->addWidget(scripts);
	layout->addWidget(akaos);
	layout->addWidget(ca);
	layout->addWidget(id);
	layout->addWidget(encounter);
	layout->addWidget(inf);
	layout->addWidget(model);
	layout->addWidget(mim);
	layout->addWidget(pathWidget);
	layout->addStretch();

	layout = new QVBoxLayout(this);
	layout->addWidget(group);
	layout->addWidget(buttonBox);
	layout->addStretch();

	connect(buttonBox, SIGNAL(accepted()), SLOT(accept()));
	connect(buttonBox, SIGNAL(rejected()), SLOT(reject()));
	connect(changePathButton, SIGNAL(clicked()), SLOT(setAdditionalPathByUser()));
	connect(mim, SIGNAL(toggled(bool)), pathWidget, SLOT(setEnabled(bool)));
}

Field::FieldSections ImportDialog::parts() const
{
	Field::FieldSections parts;

	if(scripts->isChecked())	parts |= Field::Scripts;
	if(akaos->isChecked())		parts |= Field::Akaos;
	if(ca->isChecked())			parts |= Field::Camera;
	if(id->isChecked())			parts |= Field::Walkmesh;
	if(encounter->isChecked())	parts |= Field::Encounter;
	if(inf->isChecked())		parts |= Field::Inf;
	if(model->isChecked())		parts |= Field::ModelLoader;
	if(mim->isChecked())		parts |= Field::Background;

	return parts;
}

QString ImportDialog::additionalPath() const
{
	return pathEdit->text();
}

void ImportDialog::setAdditionalPathByUser()
{
	QString path = QFileDialog::getOpenFileName(this, tr("Sélectionner le fichier MIM associé"), pathEdit->text(), tr("Fichier MIM (*.MIM);;Tous les fichiers (*)"));
	if(path.isNull()) {
		return;
	}

	pathEdit->setText(path);
}

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
#include "ImportDialog.h"

ImportDialog::ImportDialog(bool sourceSameTypeAsTarget, bool isDat, const QString &path, QWidget *parent) :
	QDialog(parent, Qt::Dialog | Qt::WindowCloseButtonHint)
{
	setWindowTitle(tr("Import"));

	QGroupBox *group = new QGroupBox(tr("Import"));
	scripts = new QCheckBox(tr("Scripts/Texts"));
	akaos = new QCheckBox(tr("Sounds/Tutorials"));
	ca = new QCheckBox(tr("Camera"));
	id = new QCheckBox(tr("Walkmesh"));
	encounter = new QCheckBox(tr("Encounters"));
	inf = new QCheckBox(tr("Triggers/gateways"));
	model = new QCheckBox(tr("Model loader"));
	mim = new QCheckBox(tr("Background"));

	notCompressed = new QCheckBox(tr("File is not compressed"));

	model->setEnabled((isDat && !sourceSameTypeAsTarget) ||
	                  (!isDat && sourceSameTypeAsTarget));
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

	if (isDat) {
		if (sourceSameTypeAsTarget) {
			model->setVisible(false);
		} else {
			model->setText(model->text() + tr(" (partial: models and animations are not linked properly)"));
			mim->setText(mim->text() + tr(" (partial: Z-depth might be a little broken)"));
		}
	}

	pathEditMim = new QLineEdit;
	QPushButton *changePathButtonMim = new QPushButton(tr("Change"));

	pathWidgetMim = new QWidget;
	QHBoxLayout *layoutPath = new QHBoxLayout(pathWidgetMim);
	layoutPath->addWidget(new QLabel(tr("MIM file:")));
	layoutPath->addWidget(pathEditMim);
	layoutPath->addWidget(changePathButtonMim);
	layoutPath->setContentsMargins(QMargins());

	pathEditBsx = new QLineEdit;
	QPushButton *changePathButtonBsx = new QPushButton(tr("Change"));

	pathWidgetBsx = new QWidget;
	QHBoxLayout *layoutPathBsx = new QHBoxLayout(pathWidgetBsx);
	layoutPathBsx->addWidget(new QLabel(tr("BSX file:")));
	layoutPathBsx->addWidget(pathEditBsx);
	layoutPathBsx->addWidget(changePathButtonBsx);
	layoutPathBsx->setContentsMargins(QMargins());

	if (isDat) {
		pathWidgetMim->setEnabled(mim->isChecked());
		pathWidgetBsx->setEnabled(model->isChecked());
		int index = path.lastIndexOf(".");
		pathEditMim->setText(index > -1 ? (path.left(index + 1) + "MIM")
		                             : path);
		pathEditBsx->setText(index > -1 ? (path.left(index + 1) + "BSX")
		                                : path);
	} else {
		pathWidgetMim->hide();
		pathWidgetBsx->hide();
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
	layout->addWidget(pathWidgetBsx);
	layout->addWidget(mim);
	layout->addWidget(pathWidgetMim);
	layout->addStretch();

	layout = new QVBoxLayout(this);
	layout->addWidget(group);
	layout->addWidget(notCompressed);
	layout->addWidget(buttonBox);
	layout->addStretch();

	connect(buttonBox, SIGNAL(accepted()), SLOT(accept()));
	connect(buttonBox, SIGNAL(rejected()), SLOT(reject()));
	connect(changePathButtonMim, SIGNAL(clicked()), SLOT(setMimPathByUser()));
	connect(changePathButtonBsx, SIGNAL(clicked()), SLOT(setBsxPathByUser()));
	connect(mim, SIGNAL(toggled(bool)), pathWidgetMim, SLOT(setEnabled(bool)));
	connect(model, SIGNAL(toggled(bool)), pathWidgetBsx, SLOT(setEnabled(bool)));
}

Field::FieldSections ImportDialog::parts() const
{
	Field::FieldSections parts;

	if (scripts->isChecked())	parts |= Field::Scripts;
	if (akaos->isChecked())		parts |= Field::Akaos;
	if (ca->isChecked())			parts |= Field::Camera;
	if (id->isChecked())			parts |= Field::Walkmesh;
	if (encounter->isChecked())	parts |= Field::Encounter;
	if (inf->isChecked())		parts |= Field::Inf;
	if (model->isChecked())		parts |= Field::ModelLoader;
	if (mim->isChecked())		parts |= Field::Background;

	return parts;
}

bool ImportDialog::isCompressed() const
{
	return ! notCompressed->isChecked();
}

QString ImportDialog::bsxPath() const
{
	return pathEditBsx->text();
}

QString ImportDialog::mimPath() const
{
	return pathEditMim->text();
}

void ImportDialog::setBsxPathByUser()
{
	QString path = QFileDialog::getOpenFileName(this, tr("Select the associated BSX file"), pathEditBsx->text(),
	                                            tr("BSX File (*.BSX);;All Files (*)"));
	if (path.isNull()) {
		return;
	}

	pathEditBsx->setText(path);
}

void ImportDialog::setMimPathByUser()
{
	QString path = QFileDialog::getOpenFileName(this, tr("Select the associated MIM file"), pathEditMim->text(),
	                                            tr("MIM File (*.MIM);;All Files (*)"));
	if (path.isNull()) {
		return;
	}

	pathEditMim->setText(path);
}

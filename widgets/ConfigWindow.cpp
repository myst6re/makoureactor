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
#include "ConfigWindow.h"
#include "Data.h"
#include "core/Config.h"
#include "TextPreview.h"

ConfigWindow::ConfigWindow(QWidget *parent)
	: QDialog(parent, Qt::Dialog | Qt::WindowCloseButtonHint)
{
	setWindowTitle(tr("Configuration"));

	QGroupBox *dependances = new QGroupBox(tr("Dépendances"), this);

	listFF7 = new QTreeWidget(dependances);
	listFF7->setColumnCount(1);
	listFF7->setUniformRowHeights(true);
	listFF7->setHeaderLabels(QStringList(tr("Final Fantasy VII installés")));
	listFF7->setFixedHeight(80);

	ff7ButtonMod = new QPushButton(dependances);
	ff7ButtonRem = new QPushButton(tr("Supprimer"), dependances);

	kernelAuto = new QCheckBox(tr("kernel2.bin"), dependances);
	kernelPath = new QLabel(dependances);
	kernelPath->setFixedWidth(500);
	kernelButton = new QPushButton(tr("Changer"), dependances);
	windowAuto = new QCheckBox(tr("window.bin"), dependances);
	windowPath = new QLabel(dependances);
	windowPath->setFixedWidth(500);
	windowButton = new QPushButton(tr("Changer"), dependances);
	charAuto = new QCheckBox(tr("char.lgp"), dependances);
	charPath = new QLabel(dependances);
	charPath->setFixedWidth(500);
	charButton = new QPushButton(tr("Changer"), dependances);

	QGridLayout *dependLayout = new QGridLayout(dependances);
	dependLayout->addWidget(listFF7, 0, 0, 3, 2);
	dependLayout->addWidget(ff7ButtonMod, 0, 2);
	dependLayout->addWidget(ff7ButtonRem, 1, 2);
	dependLayout->setRowStretch(2, 1);
	dependLayout->addWidget(kernelAuto, 3, 0);
	dependLayout->addWidget(kernelPath, 3, 1);
	dependLayout->addWidget(kernelButton, 3, 2);
	dependLayout->addWidget(windowAuto, 4, 0);
	dependLayout->addWidget(windowPath, 4, 1);
	dependLayout->addWidget(windowButton, 4, 2);
	dependLayout->addWidget(charAuto, 5, 0);
	dependLayout->addWidget(charPath, 5, 1);
	dependLayout->addWidget(charButton, 5, 2);

	QGroupBox *openGL = new QGroupBox(tr("OpenGL"), this);

	disableOGL = new QCheckBox(tr("Désactiver OpenGL"), openGL);

	QGridLayout *OGLLayout = new QGridLayout(openGL);
	OGLLayout->addWidget(disableOGL, 0, 0);

	QGroupBox *textEditor = new QGroupBox(tr("Editeur de texte"), this);

	windowColor1 = new QPushButton(textEditor);
	windowColor2 = new QPushButton(textEditor);
	windowColor3 = new QPushButton(textEditor);
	windowColor4 = new QPushButton(textEditor);
	windowPreview = new QLabel(textEditor);
	windowColorReset = new QPushButton(tr("Valeurs par défaut"), textEditor);

	optiText = new QCheckBox(trUtf8("Optimiser automatiquement les duos de caract\xc3\xa8res \xc2\xab .  \xc2\xbb, \xc2\xab .\" \xc2\xbb et \xc2\xab \xe2\x80\xa6\" \xc2\xbb."));
	optiText->hide();//TODO

	japEnc = new QCheckBox(tr("Caractères japonais"), textEditor);

	QGridLayout *textEditorLayout = new QGridLayout(textEditor);
	textEditorLayout->addWidget(windowColor1, 0, 0, Qt::AlignRight | Qt::AlignTop);
	textEditorLayout->addWidget(windowColor3, 1, 0, Qt::AlignRight | Qt::AlignBottom);
	textEditorLayout->addWidget(windowPreview, 0, 1, 2, 1, Qt::AlignCenter);
	textEditorLayout->addWidget(windowColor2, 0, 2, Qt::AlignLeft | Qt::AlignTop);
	textEditorLayout->addWidget(windowColor4, 1, 2, Qt::AlignLeft | Qt::AlignBottom);
	textEditorLayout->addWidget(windowColorReset, 2, 0, 1, 3, Qt::AlignLeft);
//	textEditorLayout->addWidget(optiText, 0, 3, Qt::AlignLeft | Qt::AlignTop);
	textEditorLayout->addWidget(japEnc, 0, 3, Qt::AlignLeft | Qt::AlignTop);
	textEditorLayout->setColumnStretch(3, 1);

	QGroupBox *misc = new QGroupBox(tr("Divers"), this);

	lzsNotCheck = new QCheckBox(tr("Ne pas vérifier strictement le format des fichiers"), misc);

	QVBoxLayout *miscLayout = new QVBoxLayout(misc);
	miscLayout->addWidget(lzsNotCheck);
	miscLayout->addStretch();

	QPushButton *OKButton = new QPushButton(tr("OK"), dependances);
	QPushButton *cancelButton = new QPushButton(tr("Annuler"), dependances);

	QGridLayout *layout = new QGridLayout(this);
	layout->addWidget(dependances, 0, 0, 1, 3);
	layout->addWidget(openGL, 1, 0, 1, 3);
	layout->addWidget(textEditor, 2, 0, 1, 3);
	layout->addWidget(misc, 3, 0, 1, 3);
	layout->addWidget(OKButton, 4, 1);
	layout->addWidget(cancelButton, 4, 2);
	layout->setColumnStretch(0, 1);

	connect(listFF7, SIGNAL(itemSelectionChanged()), SLOT(changeFF7ListButtonsState()));
	connect(ff7ButtonMod, SIGNAL(released()), SLOT(modifyCustomFF7Path()));
	connect(ff7ButtonRem, SIGNAL(released()), SLOT(removeCustomFF7Path()));
	connect(kernelAuto, SIGNAL(toggled(bool)), SLOT(kernelAutoChange(bool)));
	connect(windowAuto, SIGNAL(toggled(bool)), SLOT(windowAutoChange(bool)));
	connect(charAuto, SIGNAL(toggled(bool)), SLOT(charAutoChange(bool)));
	connect(kernelButton, SIGNAL(released()), SLOT(changeKernelPath()));
	connect(windowButton, SIGNAL(released()), SLOT(changeWindowPath()));
	connect(charButton, SIGNAL(released()), SLOT(changeCharPath()));
	connect(windowColor1, SIGNAL(released()), SLOT(changeColor()));
	connect(windowColor2, SIGNAL(released()), SLOT(changeColor()));
	connect(windowColor3, SIGNAL(released()), SLOT(changeColor()));
	connect(windowColor4, SIGNAL(released()), SLOT(changeColor()));
	connect(windowColorReset, SIGNAL(released()), SLOT(resetColor()));
	connect(OKButton, SIGNAL(released()), SLOT(accept()));
	connect(cancelButton, SIGNAL(released()), SLOT(reject()));

	fillConfig();
	changeFF7ListButtonsState();
}

void ConfigWindow::fillConfig()
{
	QString kernel_path = Config::value("kernel2Path").toString();
	QString window_path = Config::value("windowBinPath").toString();
	QString char_path = Config::value("charPath").toString();
	QString ff7_path = Data::ff7DataPath();
	QString ff7_app_path = Data::ff7AppPath();
	QStringList ff7PathList = Data::ff7AppPathList();

	listFF7->clear();
	QTreeWidgetItem *selectedItem = 0;
	int ff7PathID=0;
	foreach(const QString &ff7Path, ff7PathList) {
		if(!ff7Path.isEmpty()) {
			QTreeWidgetItem *item = new QTreeWidgetItem(QStringList(QDir::toNativeSeparators(ff7Path)));
			if(ff7Path.compare(ff7_app_path, Qt::CaseInsensitive) == 0) {
				selectedItem = item;
			}
			item->setData(0, Qt::UserRole, ff7PathID);
			listFF7->addTopLevelItem(item);
		}
		++ff7PathID;
	}
	if(selectedItem)	listFF7->setCurrentItem(selectedItem);

	if(kernel_path.isEmpty()) {
		kernel_path = Data::ff7KernelPath() % "/kernel2.bin";
		kernelAutoChange(false);
	} else {
		kernelAuto->setChecked(true);
	}

	if(window_path.isEmpty()) {
		window_path = Data::ff7KernelPath() % "/window.bin";
		windowAutoChange(false);
	} else {
		windowAuto->setChecked(true);
	}

	if(char_path.isEmpty()) {
		char_path = ff7_path % "/field/char.lgp";
		charAutoChange(false);
	} else {
		charAuto->setChecked(true);
	}

	disableOGL->setChecked(!Config::value("OpenGL", true).toBool());

	kernelPath->setText(QDir::toNativeSeparators(QDir::cleanPath(kernel_path)));
	windowPath->setText(QDir::toNativeSeparators(QDir::cleanPath(window_path)));
	charPath->setText(QDir::toNativeSeparators(QDir::cleanPath(char_path)));

	windowColorTopLeft = Config::value("windowColorTopLeft", qRgb(0,88,176)).toInt();
	windowColorTopRight = Config::value("windowColorTopRight", qRgb(0,0,80)).toInt();
	windowColorBottomLeft = Config::value("windowColorBottomLeft", qRgb(0,0,128)).toInt();
	windowColorBottomRight = Config::value("windowColorBottomRight", qRgb(0,0,32)).toInt();

	optiText->setChecked(!Config::value("dontOptimizeTexts", false).toBool());
	japEnc->setChecked(Config::value("jp_txt", false).toBool());
	lzsNotCheck->setChecked(Config::value("lzsNotCheck", false).toBool());

	setWindowColors();

	for(int j=0 ; j<listFF7->topLevelItemCount() ; ++j) {
		QCoreApplication::processEvents();
		QTreeWidgetItem *item = listFF7->topLevelItem(j);
		if(item == NULL)	break;
		item->setIcon(0, QFileIconProvider().icon(QFileInfo(item->text(0))));
	}
}

void ConfigWindow::changeFF7ListButtonsState()
{
	QTreeWidgetItem *item = listFF7->currentItem();
	if(item) {
		int id = item->data(0, Qt::UserRole).toInt();
		if(id == 2) {
			ff7ButtonRem->setEnabled(true);
			ff7ButtonMod->setText(tr("Modifier"));
		} else {
			ff7ButtonRem->setEnabled(false);
			ff7ButtonMod->setText(tr("Ajouter"));
		}
	} else {
		ff7ButtonRem->setEnabled(false);
		ff7ButtonMod->setText(tr("Ajouter"));
	}

	for(int i=0 ; i<listFF7->topLevelItemCount() ; ++i) {
		if(listFF7->topLevelItem(i)->data(0, Qt::UserRole).toInt() == 2) {
			ff7ButtonMod->setEnabled(item == listFF7->topLevelItem(i));
			return;
		}
	}
	ff7ButtonMod->setEnabled(true);
}

void ConfigWindow::modifyCustomFF7Path()
{
	QTreeWidgetItem *item = listFF7->currentItem();
	QString currentPath;
	if(item) {
		int id = item->data(0, Qt::UserRole).toInt();
		if(id == 2) {
			// Modify
			QString path = QFileDialog::getOpenFileName(this, tr("Chercher ff7.exe"), item->text(0), tr("Fichiers EXE (*.exe)"));
			if(!path.isNull()) {
				Config::setValue("customFF7Path", path);
				item->setText(0, QDir::toNativeSeparators(path));
			}
			return;
		}
		currentPath = item->text(0);
	}
	// Add
	QString path = QFileDialog::getOpenFileName(this, tr("Chercher ff7.exe"), currentPath, tr("Fichiers EXE (*.exe)"));
	if(!path.isNull()) {
		Config::setValue("customFF7Path", path);
		QTreeWidgetItem *item = new QTreeWidgetItem(QStringList(QDir::toNativeSeparators(path)));
		item->setData(0, Qt::UserRole, 2);
		item->setIcon(0, QFileIconProvider().icon(QFileInfo(path)));
		listFF7->addTopLevelItem(item);
		listFF7->setCurrentItem(item);
	}
}

void ConfigWindow::removeCustomFF7Path()
{
	QTreeWidgetItem *item = listFF7->currentItem();
	if(item) {
		int id = item->data(0, Qt::UserRole).toInt();
		if(id == 2) {
			// Remove
			Config::setValue("customFF7Path", QString());
			delete item;
			changeFF7ListButtonsState();
		}
	}
}

void ConfigWindow::kernelAutoChange(bool checked)
{
	kernelPath->setEnabled(checked);
	kernelButton->setEnabled(checked);
}

void ConfigWindow::windowAutoChange(bool checked)
{
	windowPath->setEnabled(checked);
	windowButton->setEnabled(checked);
}

void ConfigWindow::charAutoChange(bool checked)
{
	charPath->setEnabled(checked);
	charButton->setEnabled(checked);
}

void ConfigWindow::changeKernelPath()
{
	QString path = QFileDialog::getOpenFileName(this, tr("Chercher kernel2.bin"), kernelPath->text(), tr("Fichiers BIN (*.bin);;Tous les fichiers (*)"));
	if(!path.isNull())
		kernelPath->setText(QDir::toNativeSeparators(path));
}

void ConfigWindow::changeWindowPath()
{
	QString path = QFileDialog::getOpenFileName(this, tr("Chercher window.bin"), windowPath->text(), tr("Fichiers BIN (*.bin);;Tous les fichiers (*)"));
	if(!path.isNull())
		windowPath->setText(QDir::toNativeSeparators(path));
}

void ConfigWindow::changeCharPath()
{
	QString path = QFileDialog::getOpenFileName(this, tr("Chercher char.lgp"), charPath->text(), tr("Archives LGP (*.lgp);;Tous les fichiers (*)"));
	if(!path.isNull())
		charPath->setText(QDir::toNativeSeparators(path));
}

void ConfigWindow::changeColor()
{
	QColor coul;
	QRgb color=0;
	QObject *send = sender();

	if(send == windowColor1)		color = windowColorTopLeft;
	else if(send == windowColor2)	color = windowColorTopRight;
	else if(send == windowColor3)	color = windowColorBottomLeft;
	else if(send == windowColor4)	color = windowColorBottomRight;

	coul = QColorDialog::getColor(color, this);
	if(!coul.isValid())		return;

	color = coul.rgb();

	if(send == windowColor1)		windowColorTopLeft = color;
	else if(send == windowColor2)	windowColorTopRight = color;
	else if(send == windowColor3)	windowColorBottomLeft = color;
	else if(send == windowColor4)	windowColorBottomRight = color;

	setWindowColors();
}

void ConfigWindow::resetColor()
{
	windowColorTopLeft = qRgb(0,88,176);
	windowColorTopRight = qRgb(0,0,80);
	windowColorBottomLeft = qRgb(0,0,128);
	windowColorBottomRight = qRgb(0,0,32);
	setWindowColors();
}

void ConfigWindow::setWindowColorIcon(QAbstractButton *widget, QRgb color)
{
	QPixmap pix(widget->iconSize());
	pix.fill(color);
	widget->setIcon(pix);
}

void ConfigWindow::setWindowColors()
{
	setWindowColorIcon(windowColor1, windowColorTopLeft);
	setWindowColorIcon(windowColor2, windowColorTopRight);
	setWindowColorIcon(windowColor3, windowColorBottomLeft);
	setWindowColorIcon(windowColor4, windowColorBottomRight);

	int h = windowColor1->sizeHint().height()*2+10, w = 2*h;

	QPixmap pix(w, h);
	pix.fill(QColor(0, 0, 0, 0));
	QPainter p(&pix);
	TextPreview::drawWindow(&p, w, h, windowColorTopLeft, windowColorTopRight, windowColorBottomLeft, windowColorBottomRight);
	p.end();

	windowPreview->setPixmap(pix);
}

void ConfigWindow::accept()
{
	QTreeWidgetItem *currentSelectedFF7Path = listFF7->currentItem();
	int currentFF7Path = 0;
	if(currentSelectedFF7Path) {
		currentFF7Path = currentSelectedFF7Path->data(0, Qt::UserRole).toInt();
	} else if(listFF7->topLevelItemCount() > 0) {
		currentFF7Path = listFF7->topLevelItem(0)->data(0, Qt::UserRole).toInt();
	}
	Config::setValue("useRereleaseFF7Path", currentFF7Path == 1);
	Config::setValue("useCustomFF7Path", currentFF7Path == 2);
	Config::setValue("kernel2Path", kernelAuto->isChecked() ? QDir::fromNativeSeparators(kernelPath->text()) : QString());
	Config::setValue("charPath", charAuto->isChecked() ? QDir::fromNativeSeparators(charPath->text()) : QString());
	if(!disableOGL->isChecked() != Config::value("OpenGL", true).toBool()) {
		Config::setValue("OpenGL", !disableOGL->isChecked());
		QMessageBox::information(this, tr("Information"), tr("Vous devez redémarrer Makou Reactor pour appliquer tous les changements."));
	}
	Config::setValue("windowColorTopLeft", windowColorTopLeft);
	Config::setValue("windowColorTopRight", windowColorTopRight);
	Config::setValue("windowColorBottomLeft", windowColorBottomLeft);
	Config::setValue("windowColorBottomRight", windowColorBottomRight);
	Config::setValue("dontOptimizeTexts", !optiText->isChecked());
	Config::setValue("jp_txt", japEnc->isChecked());
	Config::setValue("lzsNotCheck", lzsNotCheck->isChecked());

	Data::load();//Reload kernel2.bin data
	Data::refreshFF7Paths();// refresh ff7 paths
	Data::charLgp.clear();//Refresh cached lgp TOC
	Data::charLgp.close();
	QDialog::accept();
}

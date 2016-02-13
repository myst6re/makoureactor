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
	kernelButton = new QPushButton(tr("Changer"), dependances);
	windowAuto = new QCheckBox(tr("window.bin"), dependances);
	windowPath = new QLabel(dependances);
	windowButton = new QPushButton(tr("Changer"), dependances);
	charAuto = new QCheckBox(tr("char.lgp"), dependances);
	charPath = new QLabel(dependances);
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
	dependLayout->setColumnStretch(1, 1);

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

	listCharNames = new QComboBox(textEditor);
	for(int i=0 ; i<9 ; ++i) {
		listCharNames->addItem(QIcon(QString(":/images/icon-char-%1.png").arg(i)), Data::char_names.at(i));
	}

	charNameEdit = new QLineEdit(textEditor);
	charNameEdit->setMaxLength(9);

	autoSizeMarginEdit = new QSpinBox(textEditor);
	autoSizeMarginEdit->setRange(0, 320);

	spacedCharactersWidthEdit = new QSpinBox(textEditor);
	spacedCharactersWidthEdit->setRange(0, 320);

	QGridLayout *windowPreviewLayout = new QGridLayout;
	windowPreviewLayout->addWidget(windowColor1, 0, 0, Qt::AlignRight | Qt::AlignTop);
	windowPreviewLayout->addWidget(windowColor3, 1, 0, Qt::AlignRight | Qt::AlignBottom);
	windowPreviewLayout->addWidget(windowPreview, 0, 1, 2, 1, Qt::AlignCenter);
	windowPreviewLayout->addWidget(windowColor2, 0, 2, Qt::AlignLeft | Qt::AlignTop);
	windowPreviewLayout->addWidget(windowColor4, 1, 2, Qt::AlignLeft | Qt::AlignBottom);
	windowPreviewLayout->addWidget(windowColorReset, 2, 0, 1, 3, Qt::AlignLeft);
	windowPreviewLayout->setColumnStretch(3, 1);

	QGridLayout *textEditorLayout = new QGridLayout(textEditor);
	textEditorLayout->addWidget(japEnc, 0, 0, 1, 2);
	// windowPreviewLayout->addWidget(optiText, 1, 0, 1, 2);
	textEditorLayout->addLayout(windowPreviewLayout, 1, 0, 4, 2);
	textEditorLayout->addWidget(listCharNames, 0, 2, 1, 2);
	textEditorLayout->addWidget(charNameEdit, 1, 2, 1, 2);
	textEditorLayout->addWidget(new QLabel(tr("Taille auto. : marge à droite")), 3, 2);
	textEditorLayout->addWidget(autoSizeMarginEdit, 3, 3);
	textEditorLayout->addWidget(new QLabel(tr("Largeur {SPACED CHARACTERS}")), 4, 2);
	textEditorLayout->addWidget(spacedCharactersWidthEdit, 4, 3);
	textEditorLayout->setRowStretch(2, 1);

	QGroupBox *scriptEditor = new QGroupBox(tr("Editeur de script"), this);

	expandedByDefault = new QCheckBox(tr("Lignes expansées par défaut"), scriptEditor);

	QVBoxLayout *scriptEditorLayout = new QVBoxLayout(scriptEditor);
	scriptEditorLayout->addWidget(expandedByDefault);
	scriptEditorLayout->addStretch();

	QGroupBox *misc = new QGroupBox(tr("Divers"), this);

	lzsNotCheck = new QCheckBox(tr("Ne pas vérifier strictement le format des fichiers"), misc);

	QVBoxLayout *miscLayout = new QVBoxLayout(misc);
	miscLayout->addWidget(lzsNotCheck);
	miscLayout->addStretch();

	QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, this);

	QGridLayout *layout = new QGridLayout(this);
	layout->addWidget(dependances, 0, 0, 1, 2);
	layout->addWidget(openGL, 1, 0, 1, 2);
	layout->addWidget(textEditor, 2, 0, 1, 2);
	layout->addWidget(scriptEditor, 3, 0);
	layout->addWidget(misc, 3, 1);
	layout->addWidget(buttonBox, 4, 0, 1, 2);

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
	connect(listCharNames, SIGNAL(currentIndexChanged(int)), SLOT(fillCharNameEdit()));
	connect(charNameEdit, SIGNAL(textEdited(QString)), SLOT(setCharName(QString)));
	connect(buttonBox, SIGNAL(accepted()), SLOT(accept()));
	connect(buttonBox, SIGNAL(rejected()), SLOT(reject()));

	fillConfig();
	changeFF7ListButtonsState();
}

void ConfigWindow::fillConfig()
{
	QString kernel_path = Config::value("kernel2Path").toString();
	QString window_path = Config::value("windowBinPath").toString();
	QString char_path = Config::value("charPath").toString();
	QMap<Data::FF7Version, QString> ff7PathList = Data::ff7AppPathList();

	listFF7->clear();
	QTreeWidgetItem *selectedItem = 0;
	QMapIterator<Data::FF7Version, QString> it(ff7PathList);

	while(it.hasNext()) {
		it.next();

		const QString &ff7Path = it.value();

		QTreeWidgetItem *item = new QTreeWidgetItem(QStringList(QDir::toNativeSeparators(ff7Path)));
		if(ff7Path.compare(Data::ff7AppPath(), Qt::CaseInsensitive) == 0) {
			selectedItem = item;
		}
		item->setData(0, Qt::UserRole, int(it.key()));
		listFF7->addTopLevelItem(item);
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
		char_path = Data::ff7DataPath() % "/field/char.lgp";
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
	expandedByDefault->setChecked(Config::value("scriptItemExpandedByDefault", false).toBool());
	lzsNotCheck->setChecked(Config::value("lzsNotCheck", false).toBool());

	setWindowColors();

	for(int charId=0 ; charId<9; ++charId) {
		customNames << Config::value(QString("customCharName%1").arg(charId), Data::char_names.at(charId)).toString();
	}

	fillCharNameEdit();

	autoSizeMarginEdit->setValue(Config::value("autoSizeMarginRight", 14).toInt());
	spacedCharactersWidthEdit->setValue(Config::value("spacedCharactersWidth", 13).toInt());

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
		Data::FF7Version id = Data::FF7Version(item->data(0, Qt::UserRole).toInt());
		if(id == Data::Custom) {
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
		if(listFF7->topLevelItem(i)->data(0, Qt::UserRole).toInt() == int(Data::Custom)) {
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
		Data::FF7Version id = Data::FF7Version(item->data(0, Qt::UserRole).toInt());
		if(id == Data::Custom) {
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
		item->setData(0, Qt::UserRole, int(Data::Custom));
		item->setIcon(0, QFileIconProvider().icon(QFileInfo(path)));
		listFF7->addTopLevelItem(item);
		listFF7->setCurrentItem(item);
	}
}

void ConfigWindow::removeCustomFF7Path()
{
	QTreeWidgetItem *item = listFF7->currentItem();
	if(item) {
		Data::FF7Version id = Data::FF7Version(item->data(0, Qt::UserRole).toInt());
		if(id == Data::Custom) {
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
	QString path = QFileDialog::getOpenFileName(this, tr("Chercher kernel2.bin"), QDir::fromNativeSeparators(kernelPath->text()), tr("Fichiers BIN (*.bin);;Tous les fichiers (*)"));
	if(!path.isNull())
		kernelPath->setText(QDir::toNativeSeparators(path));
}

void ConfigWindow::changeWindowPath()
{
	QString path = QFileDialog::getOpenFileName(this, tr("Chercher window.bin"), QDir::fromNativeSeparators(windowPath->text()), tr("Fichiers BIN (*.bin);;Tous les fichiers (*)"));
	if(!path.isNull())
		windowPath->setText(QDir::toNativeSeparators(path));
}

void ConfigWindow::changeCharPath()
{
	QString path = QFileDialog::getOpenFileName(this, tr("Chercher char.lgp"), QDir::fromNativeSeparators(charPath->text()), tr("Archives LGP (*.lgp);;Tous les fichiers (*)"));
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

void ConfigWindow::fillCharNameEdit()
{
	int charId = listCharNames->currentIndex();
	if(charId < 0 || charId > 8) {
		return;
	}

	charNameEdit->setText(customNames.at(charId));
}

void ConfigWindow::setCharName(const QString &charName)
{
	int charId = listCharNames->currentIndex();
	if(charId < 0 || charId > 8) {
		return;
	}

	customNames[charId] = charName;
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
	Config::setValue("FF7ExePathToUse", currentFF7Path);
	Config::setValue("kernel2Path", kernelAuto->isChecked() ? QDir::fromNativeSeparators(kernelPath->text()) : QString());
	Config::setValue("windowBinPath", windowAuto->isChecked() ? QDir::fromNativeSeparators(windowPath->text()) : QString());
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
	Config::setValue("scriptItemExpandedByDefault", expandedByDefault->isChecked());
	Config::setValue("lzsNotCheck", lzsNotCheck->isChecked());

	for(int charId=0 ; charId<9; ++charId) {
		const QString &customName = customNames.at(charId);
		if(!customName.isEmpty() && customName != Data::char_names.at(charId)) {
			Config::setValue(QString("customCharName%1").arg(charId), customName);
		} else {
			Config::remove(QString("customCharName%1").arg(charId));
		}
	}

	Config::setValue("autoSizeMarginRight", autoSizeMarginEdit->value());
	Config::setValue("spacedCharactersWidth", spacedCharactersWidthEdit->value());

	Data::loadKernel2Bin(); // Reload kernel2.bin data
	Data::loadWindowBin(); // Reload window.bin data
	Data::refreshFF7Paths(); // Refresh ff7 paths
	CharArchive::instance()->close(); // Refresh cached lgp TOC
	TextPreview::updateNames(); // Refresh custom names
	QDialog::accept();
}

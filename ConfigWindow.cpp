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

	kernelAuto = new QCheckBox(tr("Kernel2.bin"), dependances);
	kernelPath = new QLabel(dependances);
	kernelPath->setFixedWidth(500);
	kernelButton = new QPushButton(tr("Changer"), dependances);
	charAuto = new QCheckBox(tr("char.lgp"), dependances);
	charPath = new QLabel(dependances);
	charPath->setFixedWidth(500);
	charButton = new QPushButton(tr("Changer"), dependances);

	QGridLayout *dependLayout = new QGridLayout(dependances);
	dependLayout->addWidget(listFF7, 0, 0, 1, 3);
	dependLayout->addWidget(kernelAuto, 1, 0);
	dependLayout->addWidget(kernelPath, 1, 1);
	dependLayout->addWidget(kernelButton, 1, 2);
	dependLayout->addWidget(charAuto, 2, 0);
	dependLayout->addWidget(charPath, 2, 1);
	dependLayout->addWidget(charButton, 2, 2);

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

	QPushButton *OKButton = new QPushButton(tr("OK"), dependances);
	QPushButton *cancelButton = new QPushButton(tr("Annuler"), dependances);

	QGridLayout *layout = new QGridLayout(this);
	layout->addWidget(dependances, 0, 0, 1, 3);
	layout->addWidget(openGL, 1, 0, 1, 3);
	layout->addWidget(textEditor, 2, 0, 1, 3);
	layout->addWidget(OKButton, 3, 1);
	layout->addWidget(cancelButton, 3, 2);
	layout->setColumnStretch(0, 1);

	connect(kernelAuto, SIGNAL(toggled(bool)), SLOT(kernelAutoChange(bool)));
	connect(charAuto, SIGNAL(toggled(bool)), SLOT(charAutoChange(bool)));
	connect(kernelButton, SIGNAL(released()), SLOT(changeKernelPath()));
	connect(charButton, SIGNAL(released()), SLOT(changeCharPath()));
	connect(windowColor1, SIGNAL(released()), SLOT(changeColor()));
	connect(windowColor2, SIGNAL(released()), SLOT(changeColor()));
	connect(windowColor3, SIGNAL(released()), SLOT(changeColor()));
	connect(windowColor4, SIGNAL(released()), SLOT(changeColor()));
	connect(windowColorReset, SIGNAL(released()), SLOT(resetColor()));
	connect(OKButton, SIGNAL(released()), SLOT(accept()));
	connect(cancelButton, SIGNAL(released()), SLOT(reject()));

	fillConfig();
}

void ConfigWindow::fillConfig()
{
	QString kernel_path = Config::value("kernel2Path").toString();
	QString char_path = Config::value("charPath").toString();
	QString ff7_path = Data::ff7DataPath();
	QString ff7_app_path = Data::ff7AppPath();
	QStringList ff7PathList = Data::ff7AppPathList();

	listFF7->clear();
	QTreeWidgetItem *selectedItem = 0;
	foreach(const QString &ff7Path, ff7PathList) {
		QTreeWidgetItem *item = new QTreeWidgetItem(QStringList(QDir::toNativeSeparators(ff7Path)));
		if(ff7Path.compare(ff7_app_path, Qt::CaseInsensitive) == 0) {
			selectedItem = item;
		}
		listFF7->addTopLevelItem(item);
	}
	if(selectedItem)	listFF7->setCurrentItem(selectedItem);

	if(kernel_path.isEmpty()) {
		kernel_path = ff7_path%"/kernel/kernel2.bin";
		kernelAutoChange(false);
	}
	else {
		kernelAuto->setChecked(true);
	}

	if(char_path.isEmpty()) {
		char_path = ff7_path%"/field/char.lgp";
		charAutoChange(false);
	}
	else {
		charAuto->setChecked(true);
	}

	disableOGL->setChecked(!Config::value("OpenGL", true).toBool());

	kernelPath->setText(QDir::toNativeSeparators(QDir::cleanPath(kernel_path)));
	charPath->setText(QDir::toNativeSeparators(QDir::cleanPath(char_path)));

	windowColorTopLeft = Config::value("windowColorTopLeft", qRgb(0,88,176)).toInt();
	windowColorTopRight = Config::value("windowColorTopRight", qRgb(0,0,80)).toInt();
	windowColorBottomLeft = Config::value("windowColorBottomLeft", qRgb(0,0,128)).toInt();
	windowColorBottomRight = Config::value("windowColorBottomRight", qRgb(0,0,32)).toInt();

	optiText->setChecked(!Config::value("dontOptimizeTexts", false).toBool());
	japEnc->setChecked(Config::value("jp_txt", false).toBool());

	setWindowColors();

	for(int j=0 ; j<listFF7->topLevelItemCount() ; ++j) {
		QCoreApplication::processEvents();
		QTreeWidgetItem *item = listFF7->topLevelItem(j);
		if(item == NULL)	break;
		item->setIcon(0, QFileIconProvider().icon(QFileInfo(item->text(0))));
	}
}

void ConfigWindow::kernelAutoChange(bool checked)
{
	kernelPath->setEnabled(checked);
	kernelButton->setEnabled(checked);
}

void ConfigWindow::charAutoChange(bool checked)
{
	charPath->setEnabled(checked);
	charButton->setEnabled(checked);
}

void ConfigWindow::changeKernelPath()
{
	QString cheminFic = QFileDialog::getOpenFileName(this, tr("Chercher kernel2.bin"), kernelPath->text(), tr("Fichiers BIN (*.bin)"));
	if(!cheminFic.isNull())
		kernelPath->setText(QDir::toNativeSeparators(cheminFic));
}

void ConfigWindow::changeCharPath()
{
	QString cheminFic = QFileDialog::getOpenFileName(this, tr("Chercher char.lgp"), charPath->text(), tr("Archives LGP (*.lgp)"));
	if(!cheminFic.isNull())
		charPath->setText(QDir::toNativeSeparators(cheminFic));
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
	Config::setValue("useRereleaseFF7Path", currentSelectedFF7Path!=NULL && currentSelectedFF7Path == listFF7->topLevelItem(1));
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
	Data::load();//Reload kernel2.bin data
	Data::refreshFF7Paths();// refresh ff7 paths
	Data::charlgp_listPos.clear();//Refresh cached lgp TOC
	QDialog::accept();
}

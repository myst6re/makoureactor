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
#include "ConfigWindow.h"
#include "Data.h"
#include "core/Config.h"
#include "TextPreview.h"
#include "widgets/FontManager.h"
#include <DialogPreview>
#include <FF7Char>

ConfigWindow::ConfigWindow(QWidget *parent)
	: QDialog(parent, Qt::Dialog | Qt::WindowCloseButtonHint)
{
	setWindowTitle(tr("Configuration"));

	QGroupBox *dependances = new QGroupBox(tr("Dependencies"), this);

	listFF7 = new QTreeWidget(dependances);
	listFF7->setColumnCount(1);
	listFF7->setUniformRowHeights(true);
	listFF7->setHeaderLabels(QStringList(tr("Final Fantasy VII Installs")));
	listFF7->setFixedHeight(80);

	ff7ButtonMod = new QPushButton(dependances);
	ff7ButtonRem = new QPushButton(tr("Delete"), dependances);

	kernelAuto = new QCheckBox(tr("kernel2.bin"), dependances);
	kernelPath = new QLabel(dependances);
	kernelButton = new QPushButton(tr("Change"), dependances);
	windowAuto = new QCheckBox(tr("window.bin"), dependances);
	windowPath = new QLabel(dependances);
	windowButton = new QPushButton(tr("Change"), dependances);
	charAuto = new QCheckBox(tr("char.lgp"), dependances);
	charPath = new QLabel(dependances);
	charButton = new QPushButton(tr("Change"), dependances);

	encodingEdit = new QPushButton(tr("Edit..."), dependances);
	encodingEdit->setEnabled(false);

	QGridLayout *dependLayout = new QGridLayout(dependances);
	dependLayout->addWidget(listFF7, 0, 0, 3, 3);
	dependLayout->addWidget(ff7ButtonMod, 0, 3);
	dependLayout->addWidget(ff7ButtonRem, 1, 3);
	dependLayout->setRowStretch(2, 1);
	dependLayout->addWidget(kernelAuto, 3, 0);
	dependLayout->addWidget(kernelPath, 3, 1, 1, 2);
	dependLayout->addWidget(kernelButton, 3, 3);
	dependLayout->addWidget(windowAuto, 4, 0);
	dependLayout->addWidget(windowPath, 4, 1);
	dependLayout->addWidget(encodingEdit, 4, 2, Qt::AlignRight);
	dependLayout->addWidget(windowButton, 4, 3);
	dependLayout->addWidget(charAuto, 5, 0);
	dependLayout->addWidget(charPath, 5, 1, 1, 2);
	dependLayout->addWidget(charButton, 5, 3);
	dependLayout->setColumnStretch(1, 1);

	QGroupBox *theme = new QGroupBox(tr("Color Scheme"), this);

	comboPalette = new QComboBox(this);
	comboPalette->addItems({tr("System Theme"), tr("Dark Theme"), tr("Light Theme")});

	QGridLayout *themeLayout = new QGridLayout(theme);
	themeLayout->addWidget(comboPalette, 0, 0);

	QGroupBox *openGL = new QGroupBox(tr("OpenGL"), this);

	disableOGL = new QCheckBox(tr("Disable OpenGL"), openGL);

	QGridLayout *OGLLayout = new QGridLayout(openGL);
	OGLLayout->addWidget(disableOGL, 0, 0);

	QGroupBox *textEditor = new QGroupBox(tr("Text Editor"), this);

	windowPreview = new DialogPreview(textEditor);
	windowColorReset = new QPushButton(tr("Defaults"), textEditor);

	//optiText = new QCheckBox(trUtf8("Optimiser automatiquement les duos de caract\xc3\xa8res \xc2\xab .  \xc2\xbb, \xc2\xab .\" \xc2\xbb et \xc2\xab \xe2\x80\xa6\" \xc2\xbb."));
	//optiText->hide();//TODO

	encodings = new QComboBox(textEditor);
	encodings->addItem(tr("Latin"));
	encodings->addItem(tr("Japanese"));

	listCharNames = new QComboBox(textEditor);
	for (int i=0; i<9; ++i) {
		listCharNames->addItem(FF7Char::icon(i), Data::char_names.at(i));
	}

	charNameEdit = new QLineEdit(textEditor);
	charNameEdit->setMaxLength(9);

	autoSizeMarginEdit = new QSpinBox(textEditor);
	autoSizeMarginEdit->setRange(0, 320);

	spacedCharactersWidthEdit = new QSpinBox(textEditor);
	spacedCharactersWidthEdit->setRange(0, 320);

	choiceWidthEdit = new QSpinBox(textEditor);
	choiceWidthEdit->setRange(0, 320);

	tabWidthEdit = new QSpinBox(textEditor);
	tabWidthEdit->setRange(0, 320);

	QVBoxLayout *windowPreviewLayout = new QVBoxLayout;
	windowPreviewLayout->addWidget(windowPreview);
	windowPreviewLayout->addWidget(windowColorReset);

	auto windowPreviewGroup = new QGroupBox(textEditor);
	windowPreviewGroup->setTitle(tr("Dialog Background"));
	windowPreviewGroup->setLayout(windowPreviewLayout);

	QGridLayout *textEditorLayout = new QGridLayout(textEditor);
	textEditorLayout->addWidget(new QLabel(tr("Encoding")), 0, 0, 1, 3);
	textEditorLayout->addWidget(encodings, 0, 3, 1, 3);
	//textEditorLayout->addWidget(encodingEdit, 0, 4, 1, 2);
	// windowPreviewLayout->addWidget(optiText, 1, 0, 1, 2);
	textEditorLayout->addWidget(windowPreviewGroup, 1, 0, 4, 6);
	textEditorLayout->addWidget(listCharNames, 0, 6, 1, 3);
	textEditorLayout->addWidget(charNameEdit, 0, 9, 1, 3);
	textEditorLayout->addWidget(new QLabel(tr("Autosize: margin right")), 1, 6, 1, 3);
	textEditorLayout->addWidget(autoSizeMarginEdit, 1, 9, 1, 3);
	textEditorLayout->addWidget(new QLabel(tr("{SPACED CHARACTERS} width")), 2, 6, 1, 3);
	textEditorLayout->addWidget(spacedCharactersWidthEdit, 2, 9, 1, 3);
	textEditorLayout->addWidget(new QLabel(tr("{CHOICE} width")), 3, 6, 1, 3);
	textEditorLayout->addWidget(choiceWidthEdit, 3, 9, 1, 3);
	textEditorLayout->addWidget(new QLabel(tr("Tabulation width")), 4, 6, 1, 3);
	textEditorLayout->addWidget(tabWidthEdit, 4, 9, 1, 3);
	textEditorLayout->setRowStretch(2, 1);

	QGroupBox *scriptEditor = new QGroupBox(tr("Script Editor"), this);

	expandedByDefault = new QCheckBox(tr("Expand lines by default"), scriptEditor);

	QVBoxLayout *scriptEditorLayout = new QVBoxLayout(scriptEditor);
	scriptEditorLayout->addWidget(expandedByDefault);
	scriptEditorLayout->addStretch();

	QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, this);

	QGridLayout *layout = new QGridLayout(this);
	layout->addWidget(dependances, 0, 0, 1, 2);
	layout->addWidget(theme, 1, 0);
	layout->addWidget(openGL, 1, 1);
	layout->addWidget(textEditor, 2, 0, 1, 2);
	layout->addWidget(scriptEditor, 3, 0, 1, 2);
	layout->addWidget(buttonBox, 4, 0, 1, 2);

	connect(listFF7, &QTreeWidget::itemSelectionChanged, this, &ConfigWindow::changeFF7ListButtonsState);
	connect(ff7ButtonMod, &QPushButton::clicked, this, &ConfigWindow::modifyCustomFF7Path);
	connect(ff7ButtonRem, &QPushButton::clicked, this, &ConfigWindow::removeCustomFF7Path);
	connect(kernelAuto, &QCheckBox::toggled, this, &ConfigWindow::kernelAutoChange);
	connect(windowAuto, &QCheckBox::toggled, this, &ConfigWindow::windowAutoChange);
	connect(charAuto, &QCheckBox::toggled, this, &ConfigWindow::charAutoChange);
	connect(kernelButton, &QPushButton::clicked, this, &ConfigWindow::changeKernelPath);
	connect(windowButton, &QPushButton::clicked, this, &ConfigWindow::changeWindowPath);
	connect(charButton, &QPushButton::clicked, this, &ConfigWindow::changeCharPath);
	connect(windowPreview, &DialogPreview::colorChanged, this, [&] (DialogPreview::CORNER corner, const QColor &color) {
		switch(corner) {
			case DialogPreview::TOPLEFT: windowColorTopLeft = color.rgb(); break;
			case DialogPreview::BOTTOMLEFT: windowColorBottomLeft = color.rgb(); break;
			case DialogPreview::TOPRIGHT: windowColorTopRight = color.rgb(); break;
			case DialogPreview::BOTTOMRIGHT: windowColorBottomRight = color.rgb(); break;
		}
	});
	connect(windowColorReset, &QPushButton::clicked, this, &ConfigWindow::resetColor);
	connect(listCharNames, &QComboBox::currentIndexChanged, this, &ConfigWindow::fillCharNameEdit);
	connect(charNameEdit, &QLineEdit::textEdited, this, &ConfigWindow::setCharName);
	connect(encodingEdit, &QPushButton::clicked, this, &ConfigWindow::editEncoding);
	connect(buttonBox, &QDialogButtonBox::accepted, this, &ConfigWindow::accept);
	connect(buttonBox, &QDialogButtonBox::rejected, this, &ConfigWindow::reject);

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
	QTreeWidgetItem *selectedItem = nullptr;
	QMapIterator<Data::FF7Version, QString> it(ff7PathList);

	while (it.hasNext()) {
		it.next();

		const QString &ff7Path = it.value();

		QTreeWidgetItem *item = new QTreeWidgetItem(QStringList(QDir::toNativeSeparators(ff7Path)));
		if (ff7Path.compare(Data::ff7AppPath(), Qt::CaseInsensitive) == 0) {
			selectedItem = item;
		}
		item->setData(0, Qt::UserRole, int(it.key()));
		listFF7->addTopLevelItem(item);
	}
	if (selectedItem != nullptr) {
		listFF7->setCurrentItem(selectedItem);
	}

	if (kernel_path.isEmpty()) {
		kernel_path = Data::ff7KernelPath() % "/kernel2.bin";
		kernelAutoChange(false);
	} else {
		kernelAuto->setChecked(true);
	}

	if (window_path.isEmpty()) {
		window_path = Data::ff7KernelPath() % "/window.bin";
		windowAutoChange(false);
	} else {
		windowAuto->setChecked(true);
	}

	if (char_path.isEmpty()) {
		char_path = Data::ff7DataPath() % "/field/char.lgp";
		charAutoChange(false);
	} else {
		charAuto->setChecked(true);
	}

	comboPalette->setCurrentIndex(Config::value("color-scheme", 0).toInt());
	disableOGL->setChecked(!Config::value("OpenGL", true).toBool());

	kernelPath->setText(QDir::toNativeSeparators(QDir::cleanPath(kernel_path)));
	windowPath->setText(QDir::toNativeSeparators(QDir::cleanPath(window_path)));
	charPath->setText(QDir::toNativeSeparators(QDir::cleanPath(char_path)));

	encodingEdit->setDisabled(window_path.isEmpty());

	windowColorTopLeft = Config::value("windowColorTopLeft", qRgb(0,88,176)).toUInt();
	windowColorTopRight = Config::value("windowColorTopRight", qRgb(0,0,80)).toUInt();
	windowColorBottomLeft = Config::value("windowColorBottomLeft", qRgb(0,0,128)).toUInt();
	windowColorBottomRight = Config::value("windowColorBottomRight", qRgb(0,0,32)).toUInt();

	//optiText->setChecked(!Config::value("dontOptimizeTexts", false).toBool());
	encodings->setCurrentIndex(Config::value("jp_txt", false).toBool() ? 1 : 0);
	expandedByDefault->setChecked(Config::value("scriptItemExpandedByDefault", false).toBool());


	windowPreview->setColor(DialogPreview::TOPLEFT, windowColorTopLeft);
	windowPreview->setColor(DialogPreview::TOPRIGHT, windowColorTopRight);
	windowPreview->setColor(DialogPreview::BOTTOMLEFT, windowColorBottomLeft);
	windowPreview->setColor(DialogPreview::BOTTOMRIGHT, windowColorBottomRight);

	for (int charId=0; charId<9; ++charId) {
		customNames << Config::value(QString("customCharName%1").arg(charId), Data::char_names.at(charId)).toString();
	}

	fillCharNameEdit();

	autoSizeMarginEdit->setValue(Config::value("autoSizeMarginRight", 14).toInt());
	spacedCharactersWidthEdit->setValue(Config::value("spacedCharactersWidth", 13).toInt());
	choiceWidthEdit->setValue(Config::value("choiceWidth", 10).toInt());
	tabWidthEdit->setValue(Config::value("tabWidth", 4).toInt());

	QTimer::singleShot(0, this, &ConfigWindow::showIcons);
}

void ConfigWindow::showIcons()
{
	for (int j=0; j<listFF7->topLevelItemCount(); ++j) {
		QTreeWidgetItem *item = listFF7->topLevelItem(j);
		if (item == nullptr)	break;
		item->setIcon(0, QFileIconProvider().icon(QFileInfo(item->text(0))));
	}
}

void ConfigWindow::changeFF7ListButtonsState()
{
	QTreeWidgetItem *item = listFF7->currentItem();
	if (item) {
		Data::FF7Version id = Data::FF7Version(item->data(0, Qt::UserRole).toInt());
		if (id == Data::Custom) {
			ff7ButtonRem->setEnabled(true);
			ff7ButtonMod->setText(tr("Edit"));
		} else {
			ff7ButtonRem->setEnabled(false);
			ff7ButtonMod->setText(tr("Add"));
		}
	} else {
		ff7ButtonRem->setEnabled(false);
		ff7ButtonMod->setText(tr("Add"));
	}

	for (int i=0; i<listFF7->topLevelItemCount(); ++i) {
		if (listFF7->topLevelItem(i)->data(0, Qt::UserRole).toInt() == int(Data::Custom)) {
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
	if (item) {
		Data::FF7Version id = Data::FF7Version(item->data(0, Qt::UserRole).toInt());
		if (id == Data::Custom) {
			// Modify
			QString path = QFileDialog::getOpenFileName(this, tr("Find ff7.exe"), item->text(0), tr("EXE files (*.exe)"));
			if (!path.isNull()) {
				Config::setValue("customFF7Path", path);
				item->setText(0, QDir::toNativeSeparators(path));
			}
			return;
		}
		currentPath = item->text(0);
	}
	// Add
	QString path = QFileDialog::getOpenFileName(this, tr("Find ff7.exe"), currentPath, tr("EXE files (*.exe)"));
	if (!path.isNull()) {
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
	if (item) {
		Data::FF7Version id = Data::FF7Version(item->data(0, Qt::UserRole).toInt());
		if (id == Data::Custom) {
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
	QString path = QFileDialog::getOpenFileName(this, tr("Find kernel2.bin"), QDir::fromNativeSeparators(kernelPath->text()), tr("Bin Files (*.bin);;All Files (*)"));
	if (!path.isNull())
		kernelPath->setText(QDir::toNativeSeparators(path));
}

void ConfigWindow::changeWindowPath()
{
	QString path = QFileDialog::getOpenFileName(this, tr("Find window.bin"), QDir::fromNativeSeparators(windowPath->text()), tr("Bin Files (*.bin);;All Files (*)"));
	if (!path.isNull()) {
		windowPath->setText(QDir::toNativeSeparators(path));
		encodingEdit->setDisabled(path.isEmpty());
	}
}

void ConfigWindow::changeCharPath()
{
	QString path = QFileDialog::getOpenFileName(this, tr("Find char.lgp"), QDir::fromNativeSeparators(charPath->text()), tr("Lgp Archives (*.lgp);;All Files (*)"));
	if (!path.isNull())
		charPath->setText(QDir::toNativeSeparators(path));
}

void ConfigWindow::resetColor()
{
	windowColorTopLeft = qRgb(0,88,176);
	windowColorTopRight = qRgb(0,0,80);
	windowColorBottomLeft = qRgb(0,0,128);
	windowColorBottomRight = qRgb(0,0,32);
	windowPreview->setColor(DialogPreview::TOPLEFT, windowColorTopLeft);
	windowPreview->setColor(DialogPreview::TOPRIGHT, windowColorTopRight);
	windowPreview->setColor(DialogPreview::BOTTOMLEFT, windowColorBottomLeft);
	windowPreview->setColor(DialogPreview::BOTTOMRIGHT, windowColorBottomRight);
}

void ConfigWindow::fillCharNameEdit()
{
	int charId = listCharNames->currentIndex();
	if (charId < 0 || charId >= 9) {
		return;
	}

	charNameEdit->setText(customNames.at(charId));
}

void ConfigWindow::setCharName(const QString &charName)
{
	int charId = listCharNames->currentIndex();
	if (charId < 0 || charId >= 9) {
		return;
	}

	customNames[charId] = charName;
}

void ConfigWindow::editEncoding()
{
	FontManager dialog(this);
	dialog.exec();
}

void ConfigWindow::accept()
{
	bool needsRestart = false;
	QTreeWidgetItem *currentSelectedFF7Path = listFF7->currentItem();
	int currentFF7Path = 0;
	if (currentSelectedFF7Path) {
		currentFF7Path = currentSelectedFF7Path->data(0, Qt::UserRole).toInt();
	} else if (listFF7->topLevelItemCount() > 0) {
		currentFF7Path = listFF7->topLevelItem(0)->data(0, Qt::UserRole).toInt();
	}
	Config::setValue("FF7ExePathToUse", currentFF7Path);
	Config::setValue("kernel2Path", kernelAuto->isChecked() ? QDir::fromNativeSeparators(kernelPath->text()) : QString());
	Config::setValue("windowBinPath", windowAuto->isChecked() ? QDir::fromNativeSeparators(windowPath->text()) : QString());
	Config::setValue("charPath", charAuto->isChecked() ? QDir::fromNativeSeparators(charPath->text()) : QString());
	if (comboPalette->currentIndex() != Config::value("color-scheme", 0).toInt()) {
		Config::setValue("color-scheme", comboPalette->currentIndex());
		needsRestart = true;
	}
	if (!disableOGL->isChecked() != Config::value("OpenGL", true).toBool()) {
		Config::setValue("OpenGL", !disableOGL->isChecked());
		needsRestart = true;
	}
	Config::setValue("windowColorTopLeft", windowColorTopLeft);
	Config::setValue("windowColorTopRight", windowColorTopRight);
	Config::setValue("windowColorBottomLeft", windowColorBottomLeft);
	Config::setValue("windowColorBottomRight", windowColorBottomRight);
	//Config::setValue("dontOptimizeTexts", !optiText->isChecked());
	Config::setValue("jp_txt", encodings->currentIndex() == 1);
	Config::setValue("scriptItemExpandedByDefault", expandedByDefault->isChecked());

	for (int charId=0; charId<9; ++charId) {
		const QString &customName = customNames.at(charId);
		if (!customName.isEmpty() && customName != Data::char_names.at(charId)) {
			Config::setValue(QString("customCharName%1").arg(charId), customName);
		} else {
			Config::remove(QString("customCharName%1").arg(charId));
		}
	}

	Config::setValue("autoSizeMarginRight", autoSizeMarginEdit->value());
	Config::setValue("spacedCharactersWidth", spacedCharactersWidthEdit->value());
	Config::setValue("choiceWidth", choiceWidthEdit->value());
	Config::setValue("tabWidth", tabWidthEdit->value());

	if (needsRestart) {
		QMessageBox::information(this, tr("Information"), tr("You must restart %1 to apply all changes.").arg(MAKOU_REACTOR_NAME));
	}

	Data::loadKernel2Bin(); // Reload kernel2.bin data
	Data::loadWindowBin(); // Reload window.bin data
	Data::refreshFF7Paths(); // Refresh ff7 paths
	CharArchive::instance()->close(); // Refresh cached lgp TOC
	TextPreview::updateNames(); // Refresh custom names
	QDialog::accept();
}

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
#include "FontWidget.h"
#include "core/FF7Text.h"
#include "core/TexFile.h"
#include "Parameters.h"

FontWidget::FontWidget(QWidget *parent) :
	QWidget(parent), ff7Font(0)
{
	fontGrid = new FontGrid(16, 16, this);
	fontLetter = new FontLetter(this);
	fontPalette = new FontPalette(this);
	selectPal = new QComboBox(this);
	QStringList colors;
	colors << tr("Grey") << tr("Blue") << tr("Red") << tr("Violet") << tr("Green") << tr("Cyan") << tr("Yellow") << tr("White");
	selectPal->addItems(colors);
	selectPal->setCurrentIndex(selectPal->count() - 1);

	selectTable = new QComboBox(this);

	/* fromImage1 = new QPushButton(tr("From Image..."), this);
	fromImage1->setVisible(false);//TODO
	fromImage2 = new QPushButton(tr("From Image..."), this);
	fromImage2->setVisible(false);//TODO */
//	QPushButton *resetButton1 = new QPushButton(tr("Cancel Changes"), this);//TODO
	textLetter = new QLineEdit(this);
	widthLetter = new QSpinBox(this);
	widthLetter->setRange(0, 15);
	exportButton = new QPushButton(tr("Export..."), this);
	importButton = new QPushButton(tr("Import..."), this);
	resetButton2 = new QPushButton(tr("Cancel Changes"), this);
	resetButton2->setEnabled(false);

	QGridLayout *layout = new QGridLayout(this);
	layout->addWidget(fontGrid, 0, 0, 1, 2, Qt::AlignRight);
	layout->addWidget(fontLetter, 0, 2, 2, 4, Qt::AlignLeft);
	//layout->addWidget(fromImage1, 1, 0, 1, 2, Qt::AlignLeft);
	layout->addWidget(selectPal, 2, 0, Qt::AlignRight);
	layout->addWidget(selectTable, 2, 1, Qt::AlignLeft);
	layout->addWidget(fontPalette, 2, 2, 1, 4, Qt::AlignLeft);
	layout->addWidget(new QLabel(tr("Text:")), 3, 2);
	layout->addWidget(textLetter, 3, 3);
	layout->addWidget(new QLabel(tr("Width:")), 3, 4);
	layout->addWidget(widthLetter, 3, 5);
	//layout->addWidget(fromImage2, 3, 3, Qt::AlignRight);
//	layout->addWidget(resetButton1, 4, 0, 1, 2, Qt::AlignLeft);
	layout->addWidget(exportButton, 4, 0, Qt::AlignLeft);
	layout->addWidget(importButton, 4, 1, Qt::AlignLeft);
	layout->addWidget(resetButton2, 4, 2, 1, 4, Qt::AlignRight);
	layout->setRowStretch(5, 1);
	layout->setContentsMargins(QMargins());

	connect(selectPal, SIGNAL(currentIndexChanged(int)), SLOT(setColor(int)));
	connect(selectTable, SIGNAL(currentIndexChanged(int)), SLOT(setTable(int)));
	connect(fontGrid, SIGNAL(letterClicked(int)), SLOT(setLetter(int)));
	connect(fontLetter, SIGNAL(imageChanged(QRect)), fontGrid, SLOT(updateLetter(QRect)));
	connect(fontLetter, SIGNAL(imageChanged(QRect)), SLOT(setModified()));
//	connect(resetButton1, SIGNAL(clicked()), SLOT(reset()));
	connect(exportButton, SIGNAL(clicked()), SLOT(exportFont()));
	connect(importButton, SIGNAL(clicked()), SLOT(importFont()));
	connect(resetButton2, SIGNAL(clicked()), SLOT(resetLetter()));
	connect(fontPalette, SIGNAL(colorChanged(int)), fontLetter, SLOT(setPixelIndex(int)));
	connect(textLetter, SIGNAL(textEdited(QString)), SLOT(editLetter(QString)));
	connect(widthLetter, SIGNAL(valueChanged(int)), SLOT(editWidth(int)));
	connect(fontLetter, SIGNAL(widthEdited(int)), widthLetter, SLOT(setValue(int)));
}

void FontWidget::clear()
{
	fontGrid->clear();
	fontLetter->clear();
	fontPalette->clear();
}

void FontWidget::setFF7Font(FF7Font *ff7Font)
{
	this->ff7Font = ff7Font;
	setWindowBinFile(ff7Font->windowBinFile());
	setReadOnly(ff7Font->isReadOnly());
}

void FontWidget::setWindowBinFile(WindowBinFile *windowBinFile)
{
	fontGrid->setWindowBinFile(windowBinFile);
	fontLetter->setWindowBinFile(windowBinFile);
	fontPalette->setWindowBinFile(windowBinFile);
	setLetter(0);

	if (selectTable->count() != windowBinFile->tableCount()) {
		selectTable->clear();
		for (int i=1; i<=windowBinFile->tableCount(); ++i) {
			selectTable->addItem(tr("Table %1").arg(i));
		}
		selectTable->setEnabled(selectTable->count() > 1);
	}
}

void FontWidget::setReadOnly(bool ro)
{
	textLetter->setReadOnly(ro);
	fontLetter->setReadOnly(ro);
	fontPalette->setReadOnly(ro);
	/* fromImage1->setDisabled(ro);
	fromImage2->setDisabled(ro); */
}

void FontWidget::setColor(int i)
{
	fontGrid->setColor((WindowBinFile::FontColor)i);
	fontLetter->setColor((WindowBinFile::FontColor)i);
	fontPalette->setCurrentPalette((WindowBinFile::FontColor)i);
}

void FontWidget::setTable(int i)
{
	fontGrid->setCurrentTable(i);
	fontLetter->setCurrentTable(i);
	setLetter(0);
}

void FontWidget::setLetter(int i)
{
	fontLetter->setLetter(i);
	fontGrid->setLetter(i);

	QByteArray ba;

	if (fontGrid->currentTable() >= 1 && fontGrid->currentTable() <= 5) {
		ba.append(char(0xF9 + fontGrid->currentTable()));
	}

	if (fontGrid->currentTable() <= 5) {
		if (ff7Font) {
			//TODO: ff7Font
//			textLetter->setText(FF7Text(ba.append((char)i), ff7Font->tables()));
		} else {
			//TODO: jp
			textLetter->setText(FF7Text(ba.append((char)i)).text(false));
		}
		if (fontLetter->windowBinFile()) {
			widthLetter->setValue(
			    fontLetter->windowBinFile()->charWidth(
			        fontLetter->currentTable(), i
			    )
			);
		}
	}
	resetButton2->setEnabled(false);
}

void FontWidget::editLetter(const QString &letter)
{
	if (ff7Font) {
		ff7Font->setChar(
		    fontGrid->currentTable(), fontGrid->currentLetter(), letter
		);
	}
}

void FontWidget::editWidth(int w)
{
	if (fontLetter->windowBinFile() && fontLetter->windowBinFile()->charWidth(fontGrid->currentTable(), fontGrid->currentLetter()) != w) {
		fontLetter->windowBinFile()->setCharWidth(fontGrid->currentTable(), fontGrid->currentLetter(), w);
		fontLetter->update();
	}
}

void FontWidget::exportFont()
{
	WindowBinFile *windowBinFile = fontGrid->windowBinFile();

	QString binF, txtF, pngF, jpgF, bmpF;
	QStringList filter;
	if (windowBinFile) {
		filter.append(binF = tr("FF7 font file (*.bin)"));
		//filter.append(tdwF = tr("FF8 font file (*.tdw)"));
		filter.append(pngF = tr("Image File (*.png)"));
		filter.append(jpgF = tr("Image File (*.jpg)"));
		filter.append(bmpF = tr("Image File (*.bmp)"));
	}

	if (ff7Font) {
		filter.append(txtF = tr("Translation file %1 (*.txt)").arg(PROG_NAME));
	}
	QString selectedFilter;

	QString path = QFileDialog::getSaveFileName(this, tr("Export font"), "window", filter.join(";;"), &selectedFilter);
	if (path.isNull()) {
		return;
	}

	if (selectedFilter == binF) {
		QByteArray data;
		if (windowBinFile->save(data)) {
			QFile f(path);
			if (f.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
				f.write(data);
				f.close();
			} else {
				QMessageBox::warning(this, tr("Error"), tr("Error opening file (%1)").arg(f.errorString()));
			}
		} else {
			QMessageBox::warning(this, tr("Error"), tr("Error saving file"));
		}
	} /* else if (selectedFilter == tdwF) {
		// TODO: tdw
		QByteArray data;
		if (windowBinFile->save(data)) {
			QFile f(path);
			if (f.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
				f.write(data);
				f.close();
			} else {
				QMessageBox::warning(this, tr("Erreur"), tr("Erreur d'ouverture du fichier. (%1)").arg(f.errorString()));
			}
		} else {
			QMessageBox::warning(this, tr("Erreur"), tr("Erreur lors de l'enregistrement."));
		}
	} */ else if (selectedFilter == txtF) {
		QString data = ff7Font->saveTxt();
		QFile f(path);
		if (f.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
			f.write(data.toUtf8());
			f.close();
		} else {
			QMessageBox::warning(this, tr("Error"), tr("Error opening file (%1)").arg(f.errorString()));
		}
	} else if (windowBinFile &&
	          (selectedFilter == pngF ||
	           selectedFilter == jpgF ||
	           selectedFilter == bmpF)) {
		char *format;
		if (selectedFilter == pngF) {
			format = (char *)"PNG";
		} else if (selectedFilter == jpgF) {
			format = (char *)"JPG";
		} else {
			format = (char *)"BMP";
		}
		windowBinFile->image((WindowBinFile::FontColor)selectPal->currentIndex()).save(path, format);
	}
}

void FontWidget::importFont()
{
	WindowBinFile *windowBinFile = fontGrid->windowBinFile();

	QString binF;
	QStringList filter;
	if (windowBinFile) {
		filter.append(binF = tr("FF7 font file (*.bin)"));
	}
	QString selectedFilter;

	QString path = QFileDialog::getOpenFileName(this, tr("Import font"), "window", filter.join(";;"), &selectedFilter);
	if (path.isNull())		return;

	if (selectedFilter == binF) {
		QFile f(path);
		if (f.open(QIODevice::ReadOnly)) {
			WindowBinFile newWindowBinFile;
			if (newWindowBinFile.open(f.readAll())) {
				newWindowBinFile.setModified(true);
				*windowBinFile = newWindowBinFile;
				// FIXME: Update Data::windowBin?
				setWindowBinFile(windowBinFile);// update
				emit letterEdited();
			} else {
				QMessageBox::warning(this, tr("Error"), tr("Invalid file"));
			}
			f.close();
		} else {
			QMessageBox::warning(this, tr("Error"), tr("Error opening file (%1)").arg(f.errorString()));
		}
	}
}

void FontWidget::reset()
{
//	fontGrid->reset();//TODO
	fontLetter->update();
}

void FontWidget::resetLetter()
{
	fontLetter->reset();
	fontGrid->update();
	resetButton2->setEnabled(false);
}

void FontWidget::setModified()
{
	emit letterEdited();
	resetButton2->setEnabled(true);
}

void FontWidget::focusInEvent(QFocusEvent *)
{
	fontGrid->setFocus();
}

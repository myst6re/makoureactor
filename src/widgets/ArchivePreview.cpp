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
#include "ArchivePreview.h"
#include "core/Config.h"

ArchivePreview::ArchivePreview(QWidget *parent) :
      QStackedWidget(parent)
{
	setAutoFillBackground(true);
	setBackgroundRole(QPalette::Dark);

	addWidget(new QWidget(this));
	addWidget(imageWidget());
	addWidget(textWidget());
	addWidget(modelWidget());

	clearPreview();
}

QWidget *ArchivePreview::imageWidget()
{
	QWidget *ret = new QWidget(this);

	imageSelect = new QComboBox(ret);
	palSelect = new QComboBox(ret);

	scrollArea = new QScrollArea(ret);
	scrollArea->setAlignment(Qt::AlignCenter);
	scrollArea->setFrameShape(QFrame::NoFrame);

	QVBoxLayout *layout = new QVBoxLayout(ret);
	layout->setContentsMargins(QMargins());
	layout->addWidget(imageSelect, 0, Qt::AlignCenter);
	layout->addWidget(palSelect, 0, Qt::AlignCenter);
	layout->addWidget(scrollArea);

	connect(imageSelect, &QComboBox::currentIndexChanged, this, &ArchivePreview::currentImageChanged);
	connect(palSelect, &QComboBox::currentIndexChanged, this, &ArchivePreview::currentPaletteChanged);

	return ret;
}

QWidget *ArchivePreview::textWidget()
{
	QPlainTextEdit *textEdit = new QPlainTextEdit(this);
	textEdit->setReadOnly(true);
	textEdit->setFrameShape(QFrame::NoFrame);
	return textEdit;
}

QWidget *ArchivePreview::modelWidget()
{
	if (Config::value("OpenGL", true).toBool()) {
		return new FieldModel(this);
	}
	return new QWidget(this);
}

void ArchivePreview::clearPreview()
{
	setCurrentIndex(EmptyPage);
}

void ArchivePreview::imagePreview(const QPixmap &image, const QString &name,
                                   int palID, int palCount, int imageID,
                                   int imageCount)
{
	setCurrentIndex(ImagePage);
	_lbl = new ApercuBGLabel();
	_lbl->setPixmap(image);
	scrollArea->setWidget(_lbl);

	this->_name = name;

	connect(_lbl, &ApercuBGLabel::saveRequested, this, &ArchivePreview::saveImage);

	imageSelect->blockSignals(true);
	imageSelect->clear();
	if (imageCount > 1) {
		imageSelect->setVisible(true);
		for (int i=0; i<imageCount; ++i) {
			imageSelect->addItem(tr("Image %1").arg(i));
		}
		imageSelect->setCurrentIndex(imageID);
	} else {
		imageSelect->setVisible(false);
	}
	imageSelect->blockSignals(false);

	palSelect->blockSignals(true);
	palSelect->clear();
	if (palCount > 1) {
		palSelect->setVisible(true);
		for (int i=0; i<palCount; ++i) {
			palSelect->addItem(tr("Palette %1").arg(i));
		}
		palSelect->setCurrentIndex(palID);
	} else {
		palSelect->setVisible(false);
	}
	palSelect->blockSignals(false);
}

void ArchivePreview::saveImage()
{
	QString path = Config::value("saveBGPath").toString();
	if (!path.isEmpty()) {
		path.append("/");
	}
	path = QFileDialog::getSaveFileName(this, tr("Save Background"),
	                                    path + this->_name + ".png",
	                                    tr("PNG image (*.png);;JPG image (*.jpg);;BMP image (*.bmp);;Portable Pixmap (*.ppm)"));
	if (path.isNull())	return;

	_lbl->pixmap().save(path);

	int index = path.lastIndexOf('/');
	Config::setValue("saveBGPath", index == -1 ? path : path.left(index));
}

void ArchivePreview::textPreview(const QString &text)
{
	setCurrentIndex(TextPage);
	static_cast<QPlainTextEdit *>(currentWidget())->setPlainText(text);
}

void ArchivePreview::modelPreview(FieldModelFile *fieldModel)
{
	if (Config::value("OpenGL", true).toBool()) {
		setCurrentIndex(ModelPage);
		static_cast<FieldModel *>(currentWidget())->setFieldModelFile(fieldModel);
	}
}

/****************************************************************************
 ** Makou Reactor Final Fantasy VII Field Script Editor
 ** Copyright (C) 2009-2023 Arzel Jérôme <myst6re@gmail.com>
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
#include "PsColorDialog.h"
#include <PsColor>

PsColorDialog::PsColorDialog(const QColor &color, QWidget *parent) :
    QDialog(parent), _imageGrid(nullptr), _index(0)
{
	setWindowTitle(tr("Choose a color"));
	
	_topLabel = new QLabel(tr("16-bit colors:"));

	_colorPicker = new QColorPicker(this);
	_colorPicker->setFrameStyle(QFrame::Panel + QFrame::Sunken);
	_colorPicker->setCol(color.hsvHue(), color.hsvSaturation());

	_colorLumniancePicker = new QColorLuminancePicker(this);
	_colorLumniancePicker->setCol(color.hsvHue(), color.hsvSaturation(), color.value());
	_colorLumniancePicker->setFixedWidth(20);

	_colorHtmlCode = new QLineEdit(this);
	_alphaFlag = new QCheckBox(tr("Transparent"), this);

	setLayout();

	connect(_colorPicker, &QColorPicker::newCol, _colorLumniancePicker, [&](int h, int s) { _colorLumniancePicker->setCol(h, s); });
	connect(_colorLumniancePicker, &QColorLuminancePicker::newHsv, this, &PsColorDialog::setHsv);
	connect(_colorHtmlCode, &QLineEdit::editingFinished, this, [&]() { setColorFromHtmlCode(_colorHtmlCode->text()); });
	connect(_alphaFlag, &QCheckBox::toggled, this, &PsColorDialog::setTransparentEnabled);

	setColor(color);

	_alphaFlag->setChecked(color.alpha() == 0);
}

PsColorDialog::PsColorDialog(const QList<QRgb> &palette, quint8 index, QWidget *parent) :
    QDialog(parent), _colorPicker(nullptr), _colorLumniancePicker(nullptr), _colorHtmlCode(nullptr),
      _colorHtmlCodeLabel(nullptr), _alphaFlag(nullptr), _palette(palette), _index(index)
{
	setWindowTitle(tr("Choose a color"));
	
	_topLabel = new QLabel(tr("Choose a color from the associated palette:"));

	_imageGrid = new ImageGridWidget(this);
	_imageGrid->setCellSize(1);
	_imageGrid->setMinimumHeight(32 * 4);
	_imageGrid->setSelectedCell(QPoint(index % 16, index / 16));

	QImage image(16, 16, QImage::Format_ARGB32_Premultiplied);
	image.fill(Qt::black);

	for (int y = 0; y < 16; ++y) {
		for (int x = 0; x < 16; ++x) {
			image.setPixel(x, y, palette.value(x + y * 16));
		}
	}

	_imageGrid->setPixmap(QPixmap::fromImage(image));

	setLayout();

	connect(_imageGrid, &ImageGridWidget::clicked, this, &PsColorDialog::setColorFromPalette);

	setColor(_palette.value(_index));
}

void PsColorDialog::setLayout()
{
	_colorShowLabel = new QColorShowLabel(this);
	_colorShowLabel->setMinimumHeight(60);

	QDialogButtonBox *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);

	_layout = new QGridLayout(this);
	_layout->addWidget(_topLabel, 0, 0, 1, _colorPicker != nullptr ? 1 : 2);
	if (_alphaFlag != nullptr) {
		_layout->addWidget(_alphaFlag, 1, 0, 1, 2);
	}
	_layout->addWidget(_colorPicker != nullptr ? static_cast<QWidget *>(_colorPicker) : static_cast<QWidget *>(_imageGrid), 2, 0, 1, _colorPicker != nullptr ? 1 : 2);
	if (_colorLumniancePicker != nullptr) {
		_layout->addWidget(_colorLumniancePicker, 2, 1);
	}
	_layout->addWidget(_colorShowLabel, 3, 0, 1, 2);
	if (_colorHtmlCode != nullptr) {
		QHBoxLayout *layout = new QHBoxLayout;
		_colorHtmlCodeLabel = new QLabel(tr("HTML code:"), this);
		layout->addWidget(_colorHtmlCodeLabel);
		layout->addWidget(_colorHtmlCode, 1);
		_layout->addLayout(layout, 4, 0, 1, 2);
	}
	_layout->addWidget(buttons, 5, 0, 1, 2);
	_layout->setRowStretch(1, 1);
	_layout->setColumnStretch(1, 1);

	connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
	connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

void PsColorDialog::setColorFromHtmlCode(const QString &text)
{
#if QT_VERSION < QT_VERSION_CHECK(6, 4, 0)
	QColor color;
	color.setNamedColor(text);
#else
	QColor color = QColor::fromString(text);
#endif
	if (color.isValid()) {
		setColor(PsColor::fromPsColor(PsColor::toPsColor(color.rgb())));
	}
}

uint PsColorDialog::indexOrColor() const
{
	if (_imageGrid != nullptr) {
		return _index;
	}

	QRgb rgb = _color.rgb();

	if (_alphaFlag->isChecked()) {
		return qRgba(qRed(rgb), qGreen(rgb), qBlue(rgb), 0);
	}

	return qRgba(qRed(rgb), qGreen(rgb), qBlue(rgb), 255);
}

void PsColorDialog::setHsv(int h, int s, int v)
{
	setColor(QColor::fromHsv(h, s, v));
}

void PsColorDialog::setColor(const QColor &color)
{
	_color = color;
	_color.setAlpha(255);
	_colorShowLabel->setColor(_color);
	if (_colorHtmlCode != nullptr) {
		_colorHtmlCode->blockSignals(true);
		_colorHtmlCode->setText(_color.name().toUpper());
		_colorHtmlCode->blockSignals(false);
	}
}

void PsColorDialog::setColorFromPalette(const Cell &cell)
{
	_index = cell.x() + cell.y() * 16;
	setColor(_palette.value(_index));
}

void PsColorDialog::setTransparentEnabled(bool enabled)
{
	_colorPicker->setEnabled(!enabled);
	_colorLumniancePicker->setEnabled(!enabled);
	_colorShowLabel->setEnabled(!enabled);

	if (_colorHtmlCode != nullptr) {
		_colorHtmlCode->setEnabled(!enabled);
		_colorHtmlCodeLabel->setEnabled(!enabled);
	}
}

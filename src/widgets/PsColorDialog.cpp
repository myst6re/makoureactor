#include "PsColorDialog.h"

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

	setLayout();

	connect(_colorPicker, &QColorPicker::newCol, _colorLumniancePicker, [&](int h, int s) { _colorLumniancePicker->setCol(h, s); });
	connect(_colorLumniancePicker, &QColorLuminancePicker::newHsv, this, &PsColorDialog::setHsv);

	setColor(color);
}

PsColorDialog::PsColorDialog(const QList<QRgb> &palette, quint8 index, QWidget *parent) :
    QDialog(parent), _colorPicker(nullptr), _colorLumniancePicker(nullptr), _palette(palette), _index(index)
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
	_layout->addWidget(_colorPicker != nullptr ? static_cast<QWidget *>(_colorPicker) : static_cast<QWidget *>(_imageGrid), 1, 0, 1, _colorPicker != nullptr ? 1 : 2);
	if (_colorLumniancePicker != nullptr) {
		_layout->addWidget(_colorLumniancePicker, 1, 1);
	}
	_layout->addWidget(_colorShowLabel, 2, 0, 1, 2);
	_layout->addWidget(buttons, 3, 0, 1, 2);
	_layout->setRowStretch(1, 1);
	_layout->setColumnStretch(1, 1);

	connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);	
	connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

uint PsColorDialog::indexOrColor() const
{
	return _imageGrid != nullptr ? _index : _color.rgb();
}

void PsColorDialog::setHsv(int h, int s, int v)
{
	setColor(QColor::fromHsv(h, s, v));
}

void PsColorDialog::setColor(const QColor &color)
{
	_color = color;
	_colorShowLabel->setColor(_color);
}

void PsColorDialog::setColorFromPalette(const Cell &cell)
{
	_index = cell.x() + cell.y() * 16;
	setColor(_palette.value(_index));
}

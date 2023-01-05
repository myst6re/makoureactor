#include "BackgroundPaletteEditor.h"
#include "core/field/BackgroundFile.h"
#include "core/field/FieldPC.h"
#include "core/field/Palette.h"
#include "PsColorDialog.h"

BackgroundPaletteEditor::BackgroundPaletteEditor(QWidget *parent)
    : QWidget(parent), _backgroundFile(nullptr)
{
	_listWidget = new ListWidget(this);
	_listWidget->addAction(ListWidget::Add);
	_listWidget->addAction(ListWidget::Remove);

	_transparency = new QCheckBox(tr("Is first pixel transparent"), this);

	_imageGrid = new ImageGridWidget(this);
	_imageGrid->setCellSize(1);
	_imageGrid->setCursor(QCursor(Qt::CrossCursor));

	QGridLayout *layout = new QGridLayout(this);
	layout->addWidget(_listWidget, 0, 0, 2, 1);
	layout->addWidget(_transparency, 0, 1, 1, 1);
	layout->addWidget(_imageGrid, 1, 1, 1, 1);
	layout->setRowStretch(1, 1);
	layout->setColumnStretch(1, 1);

	connect(_listWidget->listWidget(), &QListWidget::currentRowChanged, this, &BackgroundPaletteEditor::setCurrentPalette);
	connect(_imageGrid, &ImageGridWidget::clicked, this, &BackgroundPaletteEditor::choosePixelColor);
	connect(_transparency, &QCheckBox::clicked, this, &BackgroundPaletteEditor::setTransparencyFlag);
}

void BackgroundPaletteEditor::setBackgroundFile(BackgroundFile *backgroundFile)
{
	_backgroundFile = backgroundFile;
	
	if (_backgroundFile == nullptr) {
		return;
	}

	_listWidget->listWidget()->blockSignals(true);
	_listWidget->listWidget()->clear();
	_listWidget->listWidget()->blockSignals(false);

	qsizetype paletteCount = _backgroundFile->palettes().size();

	for (qsizetype i = 0; i < paletteCount; ++i) {
		_listWidget->listWidget()->addItem(tr("Palette %1").arg(i));
	}

	_transparency->setVisible(_backgroundFile->field()->isPC());
}

void BackgroundPaletteEditor::clear()
{
	_listWidget->listWidget()->blockSignals(true);
	_listWidget->listWidget()->clear();
	_listWidget->listWidget()->blockSignals(false);
	_backgroundFile = nullptr;
}

void BackgroundPaletteEditor::setCurrentPalette(int palID)
{
	if (palID < 0 || _backgroundFile == nullptr || palID >= _backgroundFile->palettes().size()) {
		return;
	}

	const Palette *palette = _backgroundFile->palettes().at(palID);
	_imageGrid->setPixmap(QPixmap::fromImage(palette->toImage()));

	if (_backgroundFile->field()->isPC()) {
		const PalettePC *palettePC = static_cast<const PalettePC *>(palette);
		_transparency->setChecked(palettePC->transparency());
	}
}

Palette *BackgroundPaletteEditor::currentPalette()
{
	int palID = _listWidget->listWidget()->currentRow();

	if (palID < 0 || _backgroundFile == nullptr || palID >= _backgroundFile->palettes().size()) {
		return nullptr;
	}

	return _backgroundFile->palettes().at(palID);
}

void BackgroundPaletteEditor::choosePixelColor(const Cell &cell)
{
	Palette *palette = currentPalette();
	if (palette == nullptr) {
		return;
	}
	qsizetype i = cell.x() + cell.y() * 16;

	PsColorDialog *dialog = new PsColorDialog(palette->color(i), this);
	if (dialog->exec() == QDialog::Accepted) {
		uint indexOrColor = dialog->indexOrColor();

		palette->setColor(i, indexOrColor);

		emit modified();

		_imageGrid->setPixmap(QPixmap::fromImage(palette->toImage()));
	}
}

void BackgroundPaletteEditor::setTransparencyFlag()
{
	Palette *palette = currentPalette();
	if (palette == nullptr) {
		return;
	}

	if (_backgroundFile->field()->isPC()) {
		PalettePC *palettePC = static_cast<PalettePC *>(palette);
		palettePC->setTransparency(_transparency->isChecked());

		emit modified();
	}
}

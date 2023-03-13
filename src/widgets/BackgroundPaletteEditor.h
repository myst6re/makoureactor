#pragma once

#include <QtWidgets>
#include <ListWidget>
#include <ImageGridWidget>

class BackgroundFile;
class Palette;

class BackgroundPaletteEditor : public QWidget
{
	Q_OBJECT
public:
	explicit BackgroundPaletteEditor(QWidget *parent = nullptr);
	void setBackgroundFile(BackgroundFile *backgroundFile);
	void clear();
private slots:
	void setCurrentPalette(int palID);
	void choosePixelColor(const Cell &cell);
	void setTransparencyFlag();
	void addPalette();
	void removePalette();
signals:
	void modified();

private:
	QListWidgetItem *createItem(qsizetype i) const;
	Palette *currentPalette();

	ListWidget *_listWidget;
	QCheckBox *_transparency;
	ImageGridWidget *_imageGrid;

	BackgroundFile *_backgroundFile;
};

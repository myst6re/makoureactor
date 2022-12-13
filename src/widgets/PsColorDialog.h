#pragma once

#include <QtWidgets>
#include "QColorPicker.h"
#include "ImageGridWidget.h"

class PsColorDialog : public QDialog
{
public:
	explicit PsColorDialog(const QColor &color, QWidget *parent = nullptr);
	explicit PsColorDialog(const QList<QRgb> &palette, quint8 index, QWidget *parent = nullptr);
	uint indexOrColor() const;
private slots:
	void setColor(const QColor &color);
	void setHsv(int h, int s, int v);
	void setColorFromPalette(const Cell &cell);
private:
	void setLayout();

	QLabel *_topLabel;
	QGridLayout *_layout;
	QColorPicker *_colorPicker;
	QColorLuminancePicker *_colorLumniancePicker;
	QColorShowLabel *_colorShowLabel;
	ImageGridWidget *_imageGrid;
	QList<QRgb> _palette;
	QColor _color;
	quint8 _index;
};

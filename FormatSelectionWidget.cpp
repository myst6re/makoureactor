#include "FormatSelectionWidget.h"

FormatSelectionWidget::FormatSelectionWidget(const QString &text, const QStringList &formats, QWidget *parent) :
	QGroupBox(text, parent)
{
	setCheckable(true);
	setChecked(true);

	format = new QComboBox(this);
	format->addItems(formats);
	format->setEnabled(formats.size() > 1);

	QVBoxLayout *layout = new QVBoxLayout(this);
	layout->addWidget(format);
}

int FormatSelectionWidget::currentFormat() const
{
	return format->currentIndex();
}

void FormatSelectionWidget::setCurrentFormat(int index)
{
	format->setCurrentIndex(index);
}

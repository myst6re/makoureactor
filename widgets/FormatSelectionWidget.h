#ifndef FORMATSELECTIONWIDGET_H
#define FORMATSELECTIONWIDGET_H

#include <QtGui>

class FormatSelectionWidget : public QGroupBox
{
	Q_OBJECT
public:
	explicit FormatSelectionWidget(const QString &text, const QStringList &formats, QWidget *parent = 0);
	int currentFormat() const;
	void setCurrentFormat(int index);
private:
	QComboBox *format;
};

#endif // FORMATSELECTIONWIDGET_H

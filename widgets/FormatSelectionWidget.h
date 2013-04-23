#ifndef FORMATSELECTIONWIDGET_H
#define FORMATSELECTIONWIDGET_H

#include <QtGui>

class FormatSelectionWidget : public QGroupBox
{
	Q_OBJECT
public:
	FormatSelectionWidget(const QString &text, const QStringList &formats, QWidget *parent = 0);
	const QString &currentFormat() const;
	void setCurrentFormat(const QString &extension);
	void setFormats(const QStringList &formats);
	QStringList formats() const;
private:
	static QString splitFormatString(const QString &format, QString &extension);
	QComboBox *format;
	QStringList extensions;
};

#endif // FORMATSELECTIONWIDGET_H

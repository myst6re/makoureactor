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
#include "FormatSelectionWidget.h"

FormatSelectionWidget::FormatSelectionWidget(const QString &text, const QStringList &formats, QWidget *parent) :
	QGroupBox(text, parent)
{
	setCheckable(true);
	setChecked(true);

	format = new QComboBox(this);

	QVBoxLayout *layout = new QVBoxLayout(this);
	layout->addWidget(format);

	setFormats(formats);
}

const QString &FormatSelectionWidget::currentFormat() const
{
	return extensions.at(format->currentIndex());
}

void FormatSelectionWidget::setCurrentFormat(const QString &f)
{
	int index = extensions.indexOf(f);
	if(index != -1) {
		format->setCurrentIndex(index);
	}
}

QString FormatSelectionWidget::splitFormatString(const QString &format, QString &extension)
{
	QStringList fs = format.split(";;");
	extension = fs.size() >= 2 ? fs.at(1) : QString();
	return fs.first();
}

void FormatSelectionWidget::setFormats(const QStringList &formats)
{
	int size = qMin(formats.size(), format->count());
	QString ext;

	for(int i=0 ; i<size ; ++i) {
		const QString &f = formats.at(i);
		format->setItemText(i, splitFormatString(f, ext));
		extensions.replace(i, ext);
	}

	if(formats.size() < format->count()) {
		for(int i=format->count()-1 ; i>=formats.size() ; --i) {
			format->removeItem(i);
			extensions.removeAt(i);
		}
	} else if(formats.size() > format->count()) {
		for(int i=format->count() ; i<formats.size() ; ++i) {
			const QString &f = formats.at(i);
			format->addItem(splitFormatString(f, ext));
			extensions.append(ext);
		}
	}

	format->setEnabled(formats.size() > 1);
}

QStringList FormatSelectionWidget::formats() const
{
	QStringList formats;

	for(int i=0 ; i<format->count() ; ++i) {
		formats.append(format->itemText(i));
	}

	return formats;
}

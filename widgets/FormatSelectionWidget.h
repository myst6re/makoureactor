/****************************************************************************
 ** Makou Reactor Final Fantasy VII Field Script Editor
 ** Copyright (C) 2009-2013 Arzel Jérôme <myst6re@gmail.com>
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

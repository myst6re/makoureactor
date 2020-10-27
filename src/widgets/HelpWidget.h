/****************************************************************************
 ** Hyne Final Fantasy VIII Save Editor
 ** Copyright (C) 2013 Arzel Jérôme <myst6re@gmail.com>
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
#ifndef HELPWIDGET_H
#define HELPWIDGET_H

#include <QtWidgets>

class HelpWidget : public QWidget
{
	Q_OBJECT
public:
	enum IconType {
		IconInfo,
		IconWarning
	};

	HelpWidget(int iconExtent, const QString &text, IconType icon, QWidget *parent=0);
	HelpWidget(int iconExtent, const QString &text, QWidget *parent=0);
	HelpWidget(int iconExtent, IconType icon, QWidget *parent=0);
	explicit HelpWidget(int iconExtent, QWidget *parent=0);
	QString text() const;
	void setText(const QString &text);
private:
	void createLayout(int iconExtent, const QString &text, IconType icon = IconInfo);
	QLabel *textLabel;
};

#endif // HELPWIDGET_H

/****************************************************************************
 ** Hyne Final Fantasy VIII Save Editor
 ** Copyright (C) 2009-2022 Arzel Jérôme <myst6re@gmail.com>
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
#include "HelpWidget.h"

HelpWidget::HelpWidget(int iconExtent, const QString &text, IconType icon, QWidget *parent) :
    QWidget(parent)
{
	createLayout(iconExtent, text, icon);
}

HelpWidget::HelpWidget(int iconExtent, const QString &text, QWidget *parent) :
	QWidget(parent)
{
	createLayout(iconExtent, text);
}

HelpWidget::HelpWidget(int iconExtent, IconType icon, QWidget *parent) :
    QWidget(parent)
{
	createLayout(iconExtent, QString(), icon);
}

HelpWidget::HelpWidget(int iconExtent, QWidget *parent) :
	QWidget(parent)
{
	createLayout(iconExtent, QString());
}

void HelpWidget::createLayout(int iconExtent, const QString &text, IconType icon)
{
	textLabel = new QLabel(text, this);
	textLabel->setWordWrap(true);
	QLabel *infoIcon = new QLabel(this);
    QIcon ico = icon == IconInfo ? QIcon::fromTheme(QStringLiteral("dialog-information")) : QIcon::fromTheme(QStringLiteral("dialog-warning"));
    infoIcon->setPixmap(ico.pixmap(iconExtent));
	QHBoxLayout *infoLayout = new QHBoxLayout(this);
	infoLayout->addWidget(infoIcon);
	infoLayout->addWidget(textLabel, 1);
	infoLayout->setContentsMargins(QMargins());
}

QString HelpWidget::text() const
{
	return textLabel->text();
}

void HelpWidget::setText(const QString &text)
{
	textLabel->setText(text);
}

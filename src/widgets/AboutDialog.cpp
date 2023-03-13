/****************************************************************************
 ** Makou Reactor Final Fantasy VII Field Script Editor
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
#include "AboutDialog.h"
#include "../Data.h"
#include <ff7tkInfo>

AboutDialog::AboutDialog(QWidget *parent)
    : QDialog(parent, Qt::Dialog | Qt::CustomizeWindowHint)
{
	QLabel *image = new QLabel(this);
	image->setPixmap(QPixmap(":/images/reactor.png"));

	QLabel *desc1 = new QLabel(QString("<h1 style=\"text-align:center\">%1 %2</h1>").arg(QLatin1String(MAKOU_REACTOR_NAME), QLatin1String(MAKOU_REACTOR_VERSION))
	                           % tr("By Jérôme &lt;myst6re&gt; Arzel <br/><a href=\"https://github.com/myst6re/makoureactor/\">"
	                "github.com/myst6re/makoureactor</a>"), this);
	desc1->setTextInteractionFlags(Qt::LinksAccessibleByMouse | Qt::LinksAccessibleByKeyboard);
	desc1->setTextFormat(Qt::RichText);
	desc1->setOpenExternalLinks(true);
	
	QLabel *desc2 = new QLabel(tr("Contributors:<ul style=\"margin:0\"><li>Sithlord48</li>"
	                              "<li>TrueOdin</li><li>dangarfield</li><li>vegetass4</li><li>nickrum</li></ul>"), this);
	desc2->setTextInteractionFlags(Qt::LinksAccessibleByMouse | Qt::LinksAccessibleByKeyboard);
	desc2->setTextFormat(Qt::RichText);
	desc2->setOpenExternalLinks(true);

	QLabel *desc3 = new QLabel(tr("Thanks to:<ul style=\"margin:0\"><li>Squall78</li>"
	                              "<li>Synergy Blades</li><li>Akari</li><li>Asa</li><li>Aali</li>"
	                              "<li>DLPB</li></ul>"), this);
	desc3->setTextInteractionFlags(Qt::LinksAccessibleByMouse | Qt::LinksAccessibleByKeyboard);
	desc3->setTextFormat(Qt::RichText);
	desc3->setOpenExternalLinks(true);

	QDialogButtonBox *buttonBox = new QDialogButtonBox(Qt::Horizontal, this);
	buttonBox->addButton(QDialogButtonBox::Close);
	connect(buttonBox, &QDialogButtonBox::rejected, this, &AboutDialog::close);

	QLabel *desc4 = new QLabel(QString("Qt %1").arg(QT_VERSION_STR), this);
	QLabel *desc5 = new QLabel(QStringLiteral("ff7tk %1").arg(ff7tkInfo::version()), this);
	QPalette pal = desc4->palette();
	pal.setColor(QPalette::WindowText, Data::color(Data::ColorGreyForeground));
	desc4->setPalette(pal);
	desc5->setPalette(pal);

	QGridLayout *layout = new QGridLayout(this);
	layout->addWidget(desc1, 0, 0, 1, 2);
	layout->addWidget(desc2, 1, 0);
	layout->addWidget(image, 1, 1, 2, 1, Qt::AlignRight);
	layout->addWidget(desc3, 2, 0);
	layout->addWidget(desc4, 3, 0, 1, 2);
	layout->addWidget(desc5, 4, 0, 1, 2);
	layout->addWidget(buttonBox, 5, 0, 1, 2);
}

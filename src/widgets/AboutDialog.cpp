/****************************************************************************
 ** Makou Reactor Final Fantasy VII Field Script Editor
 ** Copyright (C) 2009-2021 Arzel Jérôme <myst6re@gmail.com>
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

AboutDialog::AboutDialog(QWidget *parent)
    : QDialog(parent, Qt::Dialog | Qt::CustomizeWindowHint)
{
	QLabel *image = new QLabel(this);
	image->setPixmap(QPixmap(":/images/reactor.png"));

	QLabel *desc1 = new QLabel(QString("<h1 style=\"text-align:center\">%1 %2</h1>").arg(MAKOU_REACTOR_NAME, MAKOU_REACTOR_VERSION) % tr("By Jérôme &lt;myst6re&gt; Arzel <br/><a href=\"https://github.com/myst6re/makoureactor/\">"
	                "github.com/myst6re/makoureactor</a>"), this);
	desc1->setTextInteractionFlags(Qt::LinksAccessibleByMouse | Qt::LinksAccessibleByKeyboard);
	desc1->setTextFormat(Qt::RichText);
	desc1->setOpenExternalLinks(true);

	QLabel *desc2 = new QLabel(tr("Thanks to:<ul style=\"margin:0\"><li>Squall78</li>"
	                              "<li>Synergy Blades</li><li>TrueOdin</li><li>Akari</li><li>Asa</li><li>Aali</li>"
	                              "<li>DLPB</li></ul>"), this);
	desc2->setTextInteractionFlags(Qt::LinksAccessibleByMouse | Qt::LinksAccessibleByKeyboard);
	desc2->setTextFormat(Qt::RichText);
	desc2->setOpenExternalLinks(true);

	QDialogButtonBox *buttonBox = new QDialogButtonBox(Qt::Horizontal, this);
	buttonBox->addButton(QDialogButtonBox::Close);
	connect(buttonBox, SIGNAL(rejected()), this, SLOT(close()));

	QLabel *desc3 = new QLabel(QString("Qt %1").arg(QT_VERSION_STR), this);
	QPalette pal = desc3->palette();
	pal.setColor(QPalette::WindowText, Data::color(Data::ColorGreyForeground));
	desc3->setPalette(pal);

	QGridLayout *layout = new QGridLayout(this);
	layout->addWidget(desc1, 0, 0, 1, 2);
	layout->addWidget(desc2, 1, 0);
	layout->addWidget(image, 1, 1, Qt::AlignRight);
	layout->addWidget(desc3, 2, 0, 1, 2);
	layout->addWidget(buttonBox, 3, 0, 1, 2);
}

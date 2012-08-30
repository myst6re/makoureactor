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
#include "ScriptEditorWindowPage.h"

ScriptEditorWindowPage::ScriptEditorWindowPage(Field *field, GrpScript *grpScript, Script *script, int opcodeID, QWidget *parent) :
	ScriptEditorView(field, grpScript, script, opcodeID, parent)
{
}

void ScriptEditorWindowPage::build()
{
	qDebug() << "build ScriptEditorWindowPage";

	textPreview = new TextPreview(this);
	winID = new QSpinBox(this);
	x = new QSpinBox(this);
	y = new QSpinBox(this);
	w = new QSpinBox(this);
	h = new QSpinBox(this);

	winID->setRange(0, 255);
	x->setRange(0, 65535);
	y->setRange(0, 65535);
	w->setRange(0, 65535);
	h->setRange(0, 65535);

	previewText = new QComboBox(this);
	previewText->addItem(tr("[Laisser la fenêtre vide]"));
	bool jp = Config::value("jp_txt", false).toBool();
	foreach(FF7Text *t, *Data::currentTextes)
		previewText->addItem(t->getText(jp, true).simplified());
	previewText->setMaximumWidth(textPreview->width()/2);

	autoSize = new QPushButton(tr("Taille auto."), this);
	autoSize->setEnabled(false);

	QGridLayout *layout = new QGridLayout(this);
	layout->addWidget(textPreview, 0, 0, 6, 2);
	layout->addWidget(new QLabel(tr("Fenêtre ID")), 0, 2);
	layout->addWidget(winID, 0, 3);
	layout->addWidget(xLabel = new QLabel(tr("X")), 1, 2);
	layout->addWidget(x, 1, 3);
	layout->addWidget(yLabel = new QLabel(tr("Y")), 2, 2);
	layout->addWidget(y, 2, 3);
	layout->addWidget(wLabel = new QLabel(tr("L")), 3, 2);
	layout->addWidget(w, 3, 3);
	layout->addWidget(hLabel = new QLabel(), 4, 2);
	layout->addWidget(h, 4, 3);
	layout->addWidget(new QLabel(tr("Texte en aperçu :")), 6, 0);
	layout->addWidget(previewText, 6, 1);
	layout->addWidget(autoSize, 6, 2, 1, 2);
	layout->setRowStretch(5, 1);
	layout->setColumnStretch(3, 1);
	layout->setContentsMargins(QMargins());

	connect(winID, SIGNAL(valueChanged(int)), SIGNAL(opcodeChanged()));
	connect(x, SIGNAL(valueChanged(int)), SLOT(updatePreview()));
	connect(y, SIGNAL(valueChanged(int)), SLOT(updatePreview()));
	connect(w, SIGNAL(valueChanged(int)), SLOT(updatePreview()));
	connect(h, SIGNAL(valueChanged(int)), SLOT(updatePreview()));
	connect(textPreview, SIGNAL(positionChanged(QPoint)), SLOT(setPositionWindow(QPoint)));
	connect(previewText, SIGNAL(currentIndexChanged(int)), SLOT(updateText(int)));
	connect(autoSize, SIGNAL(clicked()), SLOT(resizeWindow()));
}

Opcode *ScriptEditorWindowPage::opcode()
{
	if(_opcode->id() == Opcode::WROW) {
		OpcodeWROW *opcodeWROW = (OpcodeWROW *)_opcode;
		opcodeWROW->windowID = winID->value();
		opcodeWROW->rowCount = h->value();
	} else if(_opcode->id() == Opcode::WINDOW || _opcode->id() == Opcode::WSIZW) {
		OpcodeWindow *opcodeWindow = (OpcodeWindow *)_opcode;
		opcodeWindow->setWindow(textPreview->getWindow());
	}

	return ScriptEditorView::opcode();
}

void ScriptEditorWindowPage::setOpcode(Opcode *opcode)
{
	ScriptEditorView::setOpcode(opcode);
	winID->blockSignals(true);
	x->blockSignals(true);
	y->blockSignals(true);
	w->blockSignals(true);
	h->blockSignals(true);
	if(opcode->id() == Opcode::WROW) {
		xLabel->hide();
		x->hide();
		yLabel->hide();
		y->hide();
		wLabel->hide();
		w->hide();
		hLabel->setText(tr("Lignes"));
		h->setRange(0, 255);
		OpcodeWROW *opcodeWROW = (OpcodeWROW *)_opcode;
		FF7Window ff7Win = FF7Window();
		ff7Win.w = 300;
		ff7Win.h = 9 + opcodeWROW->rowCount * 16;

		textPreview->setWins(QList<FF7Window>() << ff7Win);
		winID->setValue(opcodeWROW->windowID);
		h->setValue(opcodeWROW->rowCount);
	} else if(opcode->id() == Opcode::WINDOW || opcode->id() == Opcode::WSIZW) {
		xLabel->show();
		x->show();
		yLabel->show();
		y->show();
		wLabel->show();
		w->show();
		hLabel->setText(tr("H"));
		h->setRange(0, 65535);
		OpcodeWindow *opcodeWindow = (OpcodeWindow *)_opcode;
		FF7Window ff7Win = FF7Window();
		if(opcodeWindow->getWindow(ff7Win)) {
			textPreview->setWins(QList<FF7Window>() << ff7Win);
			winID->setValue(opcodeWindow->windowID);
			x->setValue(opcodeWindow->targetX);
			y->setValue(opcodeWindow->targetY);
			w->setValue(opcodeWindow->width);
			h->setValue(opcodeWindow->height);
		}
	}
	winID->blockSignals(false);
	x->blockSignals(false);
	y->blockSignals(false);
	w->blockSignals(false);
	h->blockSignals(false);
}

void ScriptEditorWindowPage::updatePreview()
{
	FF7Window ff7Win = textPreview->getWindow();
	if(x->isVisible()) {
		ff7Win.x = x->value();
		ff7Win.y = y->value();
		ff7Win.w = w->value();
		ff7Win.h = h->value();
	} else {
		ff7Win.h = 9 + h->value() * 16;
	}
	textPreview->setWins(QList<FF7Window>() << ff7Win);
	emit opcodeChanged();
}

void ScriptEditorWindowPage::updateText(int textID)
{
	--textID;

	textPreview->setText(textID >= 0 && textID < Data::currentTextes->size()
						 ? Data::currentTextes->at(textID)->getData()
						 : QByteArray());

	autoSize->setEnabled(textID >= 0 && textID < Data::currentTextes->size());
}

void ScriptEditorWindowPage::setPositionWindow(const QPoint &point)
{
	x->blockSignals(true);
	y->blockSignals(true);
	x->setValue(point.x());
	y->setValue(point.y());
	x->blockSignals(false);
	y->blockSignals(false);
}

void ScriptEditorWindowPage::resizeWindow()
{
	textPreview->calcSize();
	QSize newSize = textPreview->getCalculatedSize();

	w->blockSignals(true);
	h->blockSignals(true);
	w->setValue(newSize.width());
	if(x->isVisible()) {
		h->setValue(newSize.height());
	} else {
		h->setValue((newSize.height() - 9) / 16);
	}
	w->blockSignals(false);
	h->blockSignals(false);

	updatePreview();
}

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
#include "ScriptEditorWindowPage.h"
#include "core/Config.h"
#include "Data.h"

ScriptEditorWindowPage::ScriptEditorWindowPage(const Section1File *scriptsAndTexts, const GrpScript &grpScript, const Script &script, int opcodeID, QWidget *parent) :
	ScriptEditorView(scriptsAndTexts, grpScript, script, opcodeID, parent)
{
}

void ScriptEditorWindowPage::build()
{
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
	previewText->addItem(tr("[Keep empty window]"));
	bool jp = Config::value("jp_txt", false).toBool();
	for (const FF7Text &t : scriptsAndTexts()->texts()) {
		previewText->addItem(previewText->fontMetrics().elidedText(t.text(jp, true).simplified(), Qt::ElideRight, 640));
	}
	previewText->setMaximumWidth(textPreview->width()/2);

	hAlign = new QPushButton(tr("Align horizontally"), this);
	vAlign = new QPushButton(tr("Align vertically"), this);
	autoSize = new QPushButton(tr("Autosize"), this);
	autoSize->setEnabled(false);

	QGridLayout *layout = new QGridLayout(this);
	layout->addWidget(textPreview, 0, 0, 8, 2);
	layout->addWidget(new QLabel(tr("Window ID")), 0, 2);
	layout->addWidget(winID, 0, 3);
	layout->addWidget(xLabel = new QLabel(tr("X")), 1, 2);
	layout->addWidget(x, 1, 3);
	layout->addWidget(yLabel = new QLabel(tr("Y")), 2, 2);
	layout->addWidget(y, 2, 3);
	layout->addWidget(wLabel = new QLabel(tr("W")), 3, 2);
	layout->addWidget(w, 3, 3);
	layout->addWidget(hLabel = new QLabel(), 4, 2);
	layout->addWidget(h, 4, 3);
	layout->addWidget(new QLabel(tr("Text in preview:")), 8, 0);
	layout->addWidget(previewText, 8, 1);
	layout->addWidget(hAlign, 6, 2, 1, 2);
	layout->addWidget(vAlign, 7, 2, 1, 2);
	layout->addWidget(autoSize, 8, 2, 1, 2);
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
	connect(hAlign, SIGNAL(clicked()), SLOT(alignHorizontally()));
	connect(vAlign, SIGNAL(clicked()), SLOT(alignVertically()));
	connect(autoSize, SIGNAL(clicked()), SLOT(resizeWindow()));
}

Opcode ScriptEditorWindowPage::buildOpcode()
{
	switch (opcode().id()) {
	case OpcodeKey::WINDOW:
	case OpcodeKey::WSIZW:
		opcode().setWindowID(quint8(winID->value()));
		opcode().setWindow(textPreview->getWindow());
		break;
	case OpcodeKey::WROW: {
		OpcodeWROW &opcodeWROW = opcode().op().opcodeWROW;
		opcodeWROW.windowID = quint8(winID->value());
		opcodeWROW.rowCount = quint8(h->value());
		break;
	} default:break;
	}

	return opcode();
}

void ScriptEditorWindowPage::setOpcode(const Opcode &opcode)
{
	ScriptEditorView::setOpcode(opcode);

	for (QObject *o : children()) {
		o->blockSignals(true);
	}
	if (opcode.id() == OpcodeKey::WROW) {
		xLabel->hide();
		x->hide();
		yLabel->hide();
		y->hide();
		wLabel->hide();
		w->hide();
		hAlign->hide();
		vAlign->hide();
		hLabel->setText(tr("Lines"));
		h->setRange(0, 255);
		const OpcodeWROW &opcodeWROW = opcode.op().opcodeWROW;
		FF7Window ff7Win = FF7Window();
		ff7Win.w = 300;
		ff7Win.h = 9 + opcodeWROW.rowCount * 16;

		textPreview->setWins(QList<FF7Window>() << ff7Win);
		winID->setValue(opcodeWROW.windowID);
		h->setValue(opcodeWROW.rowCount);
	} else if (opcode.id() == OpcodeKey::WINDOW || opcode.id() == OpcodeKey::WSIZW) {
		xLabel->show();
		x->show();
		yLabel->show();
		y->show();
		wLabel->show();
		w->show();
		hAlign->show();
		vAlign->show();
		hLabel->setText(tr("H"));
		h->setRange(0, 65535);
		FF7Window ff7Win = FF7Window();
		opcode.window(ff7Win);
		textPreview->setWins(QList<FF7Window>() << ff7Win);
		winID->setValue(opcode.windowID());
		x->setValue(ff7Win.x);
		y->setValue(ff7Win.y);
		w->setValue(ff7Win.w);
		h->setValue(ff7Win.h);
	}
	for (QObject *o : children()) {
		o->blockSignals(false);
	}

	// If the current opcode is followed by MESSAGE or ASK, we can put a text in the window preview
	if (opcodeID() + 1 < script().size()) {
		const Opcode &nextOpcode = script().opcode(opcodeID() + 1);
		if (nextOpcode.id() == OpcodeKey::MESSAGE) {
			const OpcodeMESSAGE &mess = nextOpcode.op().opcodeMESSAGE;
			if (mess.windowID == winID->value()) {
				previewText->setCurrentIndex(mess.textID + 1);
			}
		} else if (nextOpcode.id() == OpcodeKey::ASK) {
			const OpcodeASK &ask = nextOpcode.op().opcodeASK;
			if (ask.windowID == winID->value()) {
				previewText->setCurrentIndex(ask.textID + 1);
			}
		}
	}
}

void ScriptEditorWindowPage::updatePreview()
{
	FF7Window ff7Win = textPreview->getWindow();
	if (x->isVisible()) {
		ff7Win.x = qint16(x->value());
		ff7Win.y = qint16(y->value());
		ff7Win.w = quint16(w->value());
		ff7Win.h = quint16(h->value());
	} else {
		ff7Win.h = quint16(9 + h->value() * 16);
	}
	textPreview->setWins(QList<FF7Window>() << ff7Win);
	emit opcodeChanged();
}

void ScriptEditorWindowPage::updateText(int textID)
{
	--textID;
	bool hasText = textID >= 0 && textID < scriptsAndTexts()->textCount();

	textPreview->setText(hasText
						 ? scriptsAndTexts()->text(textID).data()
						 : QByteArray());

	autoSize->setEnabled(hasText);
}

void ScriptEditorWindowPage::setPositionWindow(const QPoint &point)
{
	if (x->isVisible()) {
		x->blockSignals(true);
		y->blockSignals(true);
		x->setValue(point.x());
		y->setValue(point.y());
		x->blockSignals(false);
		y->blockSignals(false);

		emit opcodeChanged();
	}
}

void ScriptEditorWindowPage::align(Qt::Alignment alignment)
{
	if (x->isVisible()) {
		x->blockSignals(true);
		y->blockSignals(true);
		if (alignment.testFlag(Qt::AlignHCenter)) {
			x->setValue((textPreview->width() - w->value()) / 2);
		}
		if (alignment.testFlag(Qt::AlignVCenter)) {
			y->setValue((textPreview->height() - h->value()) / 2);
		}
		x->blockSignals(false);
		y->blockSignals(false);

		updatePreview();
	}
}

void ScriptEditorWindowPage::resizeWindow()
{
	textPreview->calcSize();
	QSize newSize = textPreview->getCalculatedSize();

	w->blockSignals(true);
	h->blockSignals(true);
	w->setValue(newSize.width());
	if (x->isVisible()) {
		h->setValue(newSize.height());
	} else {
		h->setValue((newSize.height() - 9) / 16);
	}
	w->blockSignals(false);
	h->blockSignals(false);

	updatePreview();
}

ScriptEditorWindowModePage::ScriptEditorWindowModePage(const Section1File *scriptsAndTexts, const GrpScript &grpScript, const Script &script, int opcodeID, QWidget *parent) :
	ScriptEditorView(scriptsAndTexts, grpScript, script, opcodeID, parent)
{
}

void ScriptEditorWindowModePage::build()
{
	textPreview = new TextPreview(this);
	winID = new QSpinBox(this);
	winID->setRange(0, 255);

	winType = new QComboBox(this);
	winType->addItem(tr("Normal"));
	winType->addItem(tr("Without frame"));
	winType->addItem(tr("Transparency"));

	winClose = new QComboBox(this);
	winClose->addItem(tr("Allow"));
	winClose->addItem(tr("Prevent"));

	QGridLayout *layout = new QGridLayout(this);
	layout->addWidget(textPreview, 0, 0, 4, 1);
	layout->addWidget(new QLabel(tr("Window ID")), 0, 1);
	layout->addWidget(winID, 0, 2);
	layout->addWidget(new QLabel(tr("Type")), 1, 1);
	layout->addWidget(winType, 1, 2);
	layout->addWidget(new QLabel(tr("Closing")), 2, 1);
	layout->addWidget(winClose, 2, 2);
	layout->setRowStretch(3, 1);
	layout->setRowStretch(4, 1);
	layout->setColumnStretch(2, 1);
	layout->setContentsMargins(QMargins());

	connect(winID, SIGNAL(valueChanged(int)), SIGNAL(opcodeChanged()));
	connect(winType, SIGNAL(currentIndexChanged(int)), SLOT(updatePreview()));
	connect(winClose, SIGNAL(currentIndexChanged(int)), SIGNAL(opcodeChanged()));
}

Opcode ScriptEditorWindowModePage::buildOpcode()
{
	OpcodeWMODE &opcodeWMODE = opcode().op().opcodeWMODE;
	opcodeWMODE.windowID = quint8(winID->value());
	opcodeWMODE.mode = quint8(winType->currentIndex());
	opcodeWMODE.preventClose = quint8(winClose->currentIndex());

	return opcode();
}

void ScriptEditorWindowModePage::setOpcode(const Opcode &opcode)
{
	ScriptEditorView::setOpcode(opcode);
	for (QObject *o : children()) {
		o->blockSignals(true);
	}

	const OpcodeWMODE &opcodeWMODE = opcode.op().opcodeWMODE;
	FF7Window ff7Win = FF7Window();
	ff7Win.w = 300;
	ff7Win.h = 9 + 5 * 16;
	ff7Win.mode = opcodeWMODE.mode;

	textPreview->setWins(QList<FF7Window>() << ff7Win);
	winID->setValue(opcodeWMODE.windowID);
	winType->setCurrentIndex(opcodeWMODE.mode);
	winClose->setCurrentIndex(opcodeWMODE.preventClose);

	for (QObject *o : children()) {
		o->blockSignals(false);
	}
}

void ScriptEditorWindowModePage::updatePreview()
{
	FF7Window ff7Win = textPreview->getWindow();
	ff7Win.mode = quint8(winType->currentIndex());
	textPreview->setWins(QList<FF7Window>() << ff7Win);
	emit opcodeChanged();
}

ScriptEditorWindowMovePage::ScriptEditorWindowMovePage(const Section1File *scriptsAndTexts, const GrpScript &grpScript, const Script &script, int opcodeID, QWidget *parent) :
	ScriptEditorView(scriptsAndTexts, grpScript, script, opcodeID, parent)
{
}

void ScriptEditorWindowMovePage::build()
{
	winID = new QSpinBox(this);
	winID->setRange(0, 255);

	x = new QSpinBox(this);
	y = new QSpinBox(this);

	x->setRange(-32768, 32767);
	y->setRange(-32768, 32767);

	QGridLayout *layout = new QGridLayout(this);
	layout->addWidget(new QLabel(tr("Window ID")), 0, 0);
	layout->addWidget(winID, 0, 1);
	layout->addWidget(new QLabel(tr("Relative X")), 1, 0);
	layout->addWidget(x, 1, 1);
	layout->addWidget(new QLabel(tr("Relative Y")), 2, 0);
	layout->addWidget(y, 2, 1);
	layout->setRowStretch(3, 1);
	layout->setColumnStretch(2, 1);
	layout->setContentsMargins(QMargins());

	connect(winID, SIGNAL(valueChanged(int)), SIGNAL(opcodeChanged()));
	connect(x, SIGNAL(valueChanged(int)), SIGNAL(opcodeChanged()));
	connect(y, SIGNAL(valueChanged(int)), SIGNAL(opcodeChanged()));
}

Opcode ScriptEditorWindowMovePage::buildOpcode()
{
	OpcodeWMOVE &opcodeWMOVE = opcode().op().opcodeWMOVE;
	opcodeWMOVE.windowID = quint8(winID->value());
	opcodeWMOVE.relativeX = qint16(x->value());
	opcodeWMOVE.relativeY = qint16(y->value());

	return opcode();
}

void ScriptEditorWindowMovePage::setOpcode(const Opcode &opcode)
{
	ScriptEditorView::setOpcode(opcode);

	for (QObject *o : children()) {
		o->blockSignals(true);
	}

	const OpcodeWMOVE &opcodeWMOVE = opcode.op().opcodeWMOVE;
	winID->setValue(opcodeWMOVE.windowID);
	x->setValue(opcodeWMOVE.relativeX);
	y->setValue(opcodeWMOVE.relativeY);

	for (QObject *o : children()) {
		o->blockSignals(false);
	}
}

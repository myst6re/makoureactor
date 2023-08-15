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
#include "ScriptEditorWindowPage.h"

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
	for (const FF7String &t : scriptsAndTexts()->texts()) {
		previewText->addItem(previewText->fontMetrics().elidedText(t.text().simplified(), Qt::ElideRight, 640));
	}
	previewText->setMaximumWidth(textPreview->width() / 2);

	hAlign = new QPushButton(tr("Align horizontally"), this);
	vAlign = new QPushButton(tr("Align vertically"), this);
	autoSize = new QPushButton(tr("Autosize"), this);
	autoSize->setEnabled(false);

	QGridLayout *layout = new QGridLayout(this);
	layout->addWidget(textPreview, 0, 0, 8, 2);
	layout->addWidget(new QLabel(tr("Window ID")), 0, 2);
	layout->addWidget(winID, 0, 3);
	layout->addWidget(xLabel = new QLabel(), 1, 2);
	layout->addWidget(x, 1, 3);
	layout->addWidget(yLabel = new QLabel(tr("Y")), 2, 2);
	layout->addWidget(y, 2, 3);
	layout->addWidget(wLabel = new QLabel(tr("W")), 3, 2);
	layout->addWidget(w, 3, 3);
	layout->addWidget(hLabel = new QLabel(tr("H")), 4, 2);
	layout->addWidget(h, 4, 3);
	layout->addWidget(new QLabel(tr("Text in preview:")), 8, 0);
	layout->addWidget(previewText, 8, 1);
	layout->addWidget(hAlign, 6, 2, 1, 2);
	layout->addWidget(vAlign, 7, 2, 1, 2);
	layout->addWidget(autoSize, 8, 2, 1, 2);
	layout->setRowStretch(5, 1);
	layout->setColumnStretch(3, 1);
	layout->setContentsMargins(QMargins());

	connect(winID, &QSpinBox::valueChanged, this, &ScriptEditorWindowPage::opcodeChanged);
	connect(x, &QSpinBox::valueChanged, this, &ScriptEditorWindowPage::updatePreview);
	connect(y, &QSpinBox::valueChanged, this, &ScriptEditorWindowPage::updatePreview);
	connect(w, &QSpinBox::valueChanged, this, &ScriptEditorWindowPage::updatePreview);
	connect(h, &QSpinBox::valueChanged, this, &ScriptEditorWindowPage::updatePreview);
	connect(textPreview, &TextPreview::positionChanged, this, &ScriptEditorWindowPage::setPositionWindow);
	connect(previewText, &QComboBox::currentIndexChanged, this, &ScriptEditorWindowPage::updateText);
	connect(hAlign, &QPushButton::clicked, this, &ScriptEditorWindowPage::alignHorizontally);
	connect(vAlign, &QPushButton::clicked, this, &ScriptEditorWindowPage::alignVertically);
	connect(autoSize, &QPushButton::clicked, this, &ScriptEditorWindowPage::resizeWindow);
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
		opcodeWROW.rowCount = quint8(x->value());
		break;
	} default:break;
	}

	return opcode();
}

void ScriptEditorWindowPage::setOpcode(const Opcode &opcode)
{
	ScriptEditorView::setOpcode(opcode);
	FF7Window ff7Win = FF7Window();

	for (QObject *o : children()) {
		o->blockSignals(true);
	}
	if (opcode.id() == OpcodeKey::WROW) {
		xLabel->setText(tr("Lines"));
		x->setRange(0, 255);
		yLabel->hide();
		y->hide();
		wLabel->hide();
		w->hide();
		hLabel->hide();
		h->hide();
		hAlign->hide();
		vAlign->hide();

		const OpcodeWROW &opcodeWROW = opcode.op().opcodeWROW;
		ff7Win.w = 300;
		ff7Win.h = 9 + opcodeWROW.rowCount * 16;

		x->setValue(opcodeWROW.rowCount);
	} else if (opcode.id() == OpcodeKey::WINDOW || opcode.id() == OpcodeKey::WSIZW) {
		xLabel->setText(tr("X"));
		x->setRange(0, 65535);
		yLabel->show();
		y->show();
		wLabel->show();
		w->show();
		hLabel->show();
		h->show();
		hAlign->show();
		vAlign->show();

		FF7Window ff7Win = FF7Window();
		opcode.window(ff7Win);
		x->setValue(ff7Win.x);
		y->setValue(ff7Win.y);
		w->setValue(ff7Win.w);
		h->setValue(ff7Win.h);
	}

	textPreview->setWins(QList<FF7Window>() << ff7Win);
	textPreview->setReadOnly(!x->isVisible() || !y->isVisible());
	winID->setValue(opcode.windowID());

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
	if (h->isVisible()) {
		ff7Win.x = qint16(x->value());
		ff7Win.y = qint16(y->value());
		ff7Win.w = quint16(w->value());
		ff7Win.h = quint16(h->value());
	} else {
		ff7Win.h = quint16(9 + x->value() * 16);
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
	if (h->isVisible()) {
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
	if (h->isVisible()) {
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

	if (h->isVisible()) {
		w->blockSignals(true);
		h->blockSignals(true);
		w->setValue(newSize.width());
		h->setValue(newSize.height());
		w->blockSignals(false);
		h->blockSignals(false);
	} else {
		x->blockSignals(true);
		// Line count
		x->setValue((newSize.height() - 9) / 16);
		x->blockSignals(false);
	}

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
	layout->setColumnStretch(2, 1);
	layout->setContentsMargins(QMargins());

	connect(winID, &QSpinBox::valueChanged, this, &ScriptEditorWindowModePage::opcodeChanged);
	connect(winType, &QComboBox::currentIndexChanged, this, &ScriptEditorWindowModePage::updatePreview);
	connect(winClose, &QComboBox::currentIndexChanged, this, &ScriptEditorWindowModePage::opcodeChanged);
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

	connect(winID, &QSpinBox::valueChanged, this, &ScriptEditorWindowMovePage::opcodeChanged);
	connect(x, &QSpinBox::valueChanged, this, &ScriptEditorWindowMovePage::opcodeChanged);
	connect(y, &QSpinBox::valueChanged, this, &ScriptEditorWindowMovePage::opcodeChanged);
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

ScriptEditorWindowVariablePage::ScriptEditorWindowVariablePage(const Section1File *scriptsAndTexts, const GrpScript &grpScript, const Script &script, int opcodeID, QWidget *parent) :
	ScriptEditorView(scriptsAndTexts, grpScript, script, opcodeID, parent)
{
}

void ScriptEditorWindowVariablePage::build()
{
	winID = new QSpinBox(this);
	winID->setRange(0, 255);
	winVar = new QSpinBox(this);
	winVar->setRange(0, 255);

	varOrValue = new VarOrValueWidget(this);
	varOrValue->setShortAndLong();
	varOrValue->setSignedValueType(true);

	QGridLayout *layout = new QGridLayout(this);
	layout->addWidget(new QLabel(tr("Window ID")), 0, 0);
	layout->addWidget(winID, 0, 1);
	layout->addWidget(new QLabel(tr("Window Variable")), 1, 0);
	layout->addWidget(winVar, 1, 1);
	layout->addWidget(new QLabel(tr("Value")), 2, 0);
	layout->addWidget(varOrValue, 2, 1);
	layout->setRowStretch(3, 1);
	layout->setColumnStretch(2, 1);
	layout->setContentsMargins(QMargins());

	connect(winID, &QSpinBox::valueChanged, this, &ScriptEditorWindowVariablePage::opcodeChanged);
	connect(winVar, &QSpinBox::valueChanged, this, &ScriptEditorWindowVariablePage::opcodeChanged);
	connect(varOrValue, &VarOrValueWidget::changed, this, &ScriptEditorWindowVariablePage::opcodeChanged);
}

Opcode ScriptEditorWindowVariablePage::buildOpcode()
{
	quint8 bank;
	int value;
	OpcodeKey key = opcode().id();
	
	if (varOrValue->isValue()) {
		bank = 0;
		value = varOrValue->value();
		
		if (value > 255 || value < 0) {
			key = OpcodeKey::MPRA2;
		}
	} else {
		quint8 address;
		varOrValue->var(bank, address);
		value = address;

		if (varOrValue->selectedSize() == VarOrValueWidget::Long) {
			key = OpcodeKey::MPRA2;
		}
	}

	if (key == OpcodeKey::MPARA) {
		OpcodeMPARA &opcodeMPARA = opcode().op().opcodeMPARA;
		opcodeMPARA.id = OpcodeKey::MPARA;
		opcodeMPARA.banks = BANK(0, bank);
		opcodeMPARA.windowID = quint8(winID->value());
		opcodeMPARA.windowVarID = quint8(winVar->value());
		opcodeMPARA.value = quint8(value);
	} else {
		OpcodeMPRA2 &opcodeMPRA2 = opcode().op().opcodeMPRA2;
		opcodeMPRA2.id = OpcodeKey::MPRA2;
		opcodeMPRA2.banks = BANK(0, bank);
		opcodeMPRA2.windowID = quint8(winID->value());
		opcodeMPRA2.windowVarID = quint8(winVar->value());
		opcodeMPRA2.value = quint16(value);
	}

	return opcode();
}

void ScriptEditorWindowVariablePage::setOpcode(const Opcode &opcode)
{
	ScriptEditorView::setOpcode(opcode);

	for (QObject *o : children()) {
		o->blockSignals(true);
	}

	if (opcode.id() == OpcodeKey::MPARA) {
		const OpcodeMPARA &opcodeMPARA = opcode.op().opcodeMPARA;
		winID->setValue(opcodeMPARA.windowID);
		winVar->setValue(opcodeMPARA.windowVarID);
		varOrValue->setVarOrValue(B2(opcodeMPARA.banks), opcodeMPARA.value);
	} else {
		const OpcodeMPRA2 &opcodeMPRA2 = opcode.op().opcodeMPRA2;
		winID->setValue(opcodeMPRA2.windowID);
		winVar->setValue(opcodeMPRA2.windowVarID);
		varOrValue->setVarOrValue(B2(opcodeMPRA2.banks), opcodeMPRA2.value);
	}

	for (QObject *o : children()) {
		o->blockSignals(false);
	}
}

ScriptEditorWindowNumDisplayPage::ScriptEditorWindowNumDisplayPage(const Section1File *scriptsAndTexts, const GrpScript &grpScript, const Script &script, int opcodeID, QWidget *parent) :
	ScriptEditorView(scriptsAndTexts, grpScript, script, opcodeID, parent)
{
}

void ScriptEditorWindowNumDisplayPage::build()
{
	textPreview = new TextPreview(this);

	displayMode = new QComboBox(this);
	displayMode->addItem(tr("None"));
	displayMode->addItem(tr("Clock (00:00)"));
	displayMode->addItem(tr("Numerical (00)"));

	winID = new QSpinBox(this);
	winID->setRange(0, 255);
	relativeX = new QSpinBox(this);
	relativeX->setRange(0, 255);
	relativeY = new QSpinBox(this);
	relativeY->setRange(0, 255);

	previewText = new QComboBox(this);
	previewText->setMaximumWidth(textPreview->width() / 2);

	previewWindow = new QComboBox(this);
	previewWindow->setMaximumWidth(textPreview->width() / 2);

	QGridLayout *layout = new QGridLayout(this);
	layout->addWidget(textPreview, 0, 0, 5, 2);
	layout->addWidget(new QLabel(tr("Text in preview:")), 6, 0);
	layout->addWidget(previewText, 6, 1);
	layout->addWidget(new QLabel(tr("Window in preview:")), 7, 0);
	layout->addWidget(previewWindow, 7, 1);
	layout->addWidget(new QLabel(tr("Window ID")), 0, 2);
	layout->addWidget(winID, 0, 3);
	layout->addWidget(new QLabel(tr("Display Mode")), 1, 2);
	layout->addWidget(displayMode, 1, 3);
	layout->addWidget(new QLabel(tr("Margin left")), 2, 2);
	layout->addWidget(relativeX, 2, 3);
	layout->addWidget(new QLabel(tr("Margin top")), 3, 2);
	layout->addWidget(relativeY, 3, 3);
	layout->setRowStretch(4, 1);
	layout->setColumnStretch(3, 1);
	layout->setContentsMargins(QMargins());

	connect(winID, &QSpinBox::valueChanged, this, &ScriptEditorWindowNumDisplayPage::opcodeChanged);
	connect(winID, &QSpinBox::valueChanged, this, &ScriptEditorWindowNumDisplayPage::updatePreviewTextList);
	connect(winID, &QSpinBox::valueChanged, this, &ScriptEditorWindowNumDisplayPage::updatePreviewWindowList);
	connect(displayMode, &QComboBox::currentIndexChanged, this, &ScriptEditorWindowNumDisplayPage::opcodeChanged);
	connect(relativeX, &QSpinBox::valueChanged, this, &ScriptEditorWindowNumDisplayPage::opcodeChanged);
	connect(relativeY, &QSpinBox::valueChanged, this, &ScriptEditorWindowNumDisplayPage::opcodeChanged);
	connect(displayMode, &QComboBox::currentIndexChanged, this, &ScriptEditorWindowNumDisplayPage::updatePreview);
	connect(relativeX, &QSpinBox::valueChanged, this, &ScriptEditorWindowNumDisplayPage::updatePreview);
	connect(relativeY, &QSpinBox::valueChanged, this, &ScriptEditorWindowNumDisplayPage::updatePreview);
	connect(previewText, &QComboBox::currentIndexChanged, this, &ScriptEditorWindowNumDisplayPage::updatePreviewWindowList);
	connect(previewText, &QComboBox::currentIndexChanged, this, &ScriptEditorWindowNumDisplayPage::updatePreview);
	connect(previewWindow, &QComboBox::currentIndexChanged, this, &ScriptEditorWindowNumDisplayPage::updatePreview);
}

Opcode ScriptEditorWindowNumDisplayPage::buildOpcode()
{
	OpcodeWSPCL &opcodeWSPCL = opcode().op().opcodeWSPCL;

	opcodeWSPCL.windowID = quint8(winID->value());
	opcodeWSPCL.displayType = quint8(displayMode->currentIndex());
	opcodeWSPCL.marginLeft = quint8(relativeX->value());
	opcodeWSPCL.marginTop = quint8(relativeY->value());

	return opcode();
}

void ScriptEditorWindowNumDisplayPage::setOpcode(const Opcode &opcode)
{
	ScriptEditorView::setOpcode(opcode);
	
	const OpcodeWSPCL &opcodeWSPCL = opcode.op().opcodeWSPCL; 

	for (QObject *o : children()) {
		o->blockSignals(true);
	}

	winID->setValue(opcodeWSPCL.windowID);
	displayMode->setCurrentIndex(opcodeWSPCL.displayType);
	relativeX->setValue(opcodeWSPCL.marginLeft);
	relativeY->setValue(opcodeWSPCL.marginTop);

	textPreview->setReadOnly(false);
	updatePreviewTextList();
	updatePreviewWindowList();
	updatePreview();

	for (QObject *o : children()) {
		o->blockSignals(false);
	}
}

void ScriptEditorWindowNumDisplayPage::updatePreview()
{
	FF7Window ff7Win = FF7Window();

	int textID = previewText->currentData().toInt();

	if (textID >= 0) {
		QList<FF7Window> windows;
		scriptsAndTexts()->listWindows(textID, windows, winID->value());

		if (previewWindow->currentIndex() >= 0 && previewWindow->currentIndex() < windows.size()) {
			ff7Win = windows.at(previewWindow->currentIndex());
		} else {
			ff7Win.mode = 0x01;
		}
		textPreview->setText(scriptsAndTexts()->text(textID).data());
	} else {
		ff7Win.mode = 0x01;
	}

	ff7Win.displayType = quint8(displayMode->currentIndex());
	ff7Win.displayX = quint8(relativeX->value());
	ff7Win.displayY = quint8(relativeY->value());
	textPreview->setWins(QList<FF7Window>() << ff7Win);
}

void ScriptEditorWindowNumDisplayPage::updatePreviewTextList()
{
	previewText->clear();

	QMultiMap<quint64, FF7Window> windows;
	QMultiMap<quint8, quint64> text2win;
	
	script().listWindows(0, 0, windows, text2win);

	quint8 textID = 0, windowID = quint8(winID->value());
	for (const FF7String &t : scriptsAndTexts()->texts()) {
		if (text2win.contains(textID) && (text2win.value(textID) & 0xFF) == windowID) {
			previewText->addItem(previewText->fontMetrics().elidedText(t.text().simplified(), Qt::ElideRight, 640), textID);
		}
		textID += 1;
	}

	previewText->setEnabled(previewText->count() > 0);
}

void ScriptEditorWindowNumDisplayPage::updatePreviewWindowList()
{
	previewWindow->clear();

	int textID = previewText->currentData().toInt();
	qDebug() << "updatePreviewWindowList" << textID;

	if (textID >= 0) {
		QList<FF7Window> windows;
		script().listWindows(0, 0, textID, windows, winID->value());

		for (const FF7Window &win: windows) {
			previewWindow->addItem(tr("Window %1 (x: %2, y: %3, w: %4, h: %5)")
			                       .arg(winID->value())
			                       .arg(win.x)
			                       .arg(win.y)
			                       .arg(win.w)
			                       .arg(win.h));
		}
	}

	previewWindow->setEnabled(previewWindow->count() > 0);
}

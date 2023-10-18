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
#include "ScriptEditorStructPage.h"
#include "Data.h"

ScriptEditorWithPriorityPage::ScriptEditorWithPriorityPage(const Section1File *scriptsAndTexts, const GrpScript &grpScript, const Script &script, int opcodeID, QWidget *parent) :
      ScriptEditorView(scriptsAndTexts, grpScript, script, opcodeID, parent)
{
}

void ScriptEditorWithPriorityPage::build()
{
	priority = new QSpinBox(this);
	priority->setRange(0, 6);

	helpWidget = new HelpWidget(16, tr("Lower priority number is higher priority in the game"));

	connect(priority, &QSpinBox::valueChanged, this, &ScriptEditorWithPriorityPage::opcodeChanged);
}

ScriptEditorReturnToPage::ScriptEditorReturnToPage(const Section1File *scriptsAndTexts, const GrpScript &grpScript, const Script &script, int opcodeID, QWidget *parent) :
	ScriptEditorWithPriorityPage(scriptsAndTexts, grpScript, script, opcodeID, parent)
{
}

void ScriptEditorReturnToPage::build()
{
	ScriptEditorWithPriorityPage::build();

	scriptList = new QComboBox(this);
	for (quint8 i = 0; i < 32; ++i) {
		scriptList->addItem(tr("Script %1").arg(i));
	}

	QGridLayout *layout = new QGridLayout(this);
	layout->addWidget(new QLabel(tr("Script")), 0, 0);
	layout->addWidget(scriptList, 0, 1, 1, 3);
	layout->addWidget(new QLabel(tr("Priority")), 1, 0);
	layout->addWidget(priority, 1, 1, 1, 3);
	layout->addWidget(helpWidget, 2, 0, 1, 4);
	layout->setRowStretch(3, 1);
	layout->setContentsMargins(QMargins());

	connect(scriptList, &QComboBox::currentIndexChanged, this, &ScriptEditorReturnToPage::opcodeChanged);
}

Opcode ScriptEditorReturnToPage::buildOpcode()
{
	opcode().setScriptIDAndPriority(quint8(scriptList->currentIndex()), quint8(priority->value()));

	return opcode();
}

void ScriptEditorReturnToPage::setOpcode(const Opcode &opcode)
{
	ScriptEditorView::setOpcode(opcode);

	for (QObject *o : children()) {
		o->blockSignals(true);
	}

	const OpcodeRETTO &opcodeRETTO = opcode.op().opcodeRETTO;
	scriptList->setCurrentIndex(SCRIPT_ID(opcodeRETTO.scriptIDAndPriority));
	priority->setValue(std::min(6, PRIORITY(opcodeRETTO.scriptIDAndPriority)));

	for (QObject *o : children()) {
		o->blockSignals(false);
	}
}

ScriptEditorExecPage::ScriptEditorExecPage(const Section1File *scriptsAndTexts, const GrpScript &grpScript, const Script &script, int opcodeID, QWidget *parent) :
	ScriptEditorWithPriorityPage(scriptsAndTexts, grpScript, script, opcodeID, parent)
{
}

void ScriptEditorExecPage::build()
{
	ScriptEditorWithPriorityPage::build();

	groupList = new QComboBox(this);
	qsizetype i = 0;
	for (const GrpScript &group : scriptsAndTexts()->grpScripts()) {
		groupList->addItem(QString("%1 - %2").arg(i++).arg(group.name()));
	}

	scriptList = new QComboBox(this);
	for (quint8 i = 0; i < 32; ++i) {
		scriptList->addItem("");
	}

	execType = new QComboBox(this);
	execType->addItem(tr("Asynchronous, no wait"));
	execType->addItem(tr("Asynchronous, wait"));
	execType->addItem(tr("Synchronous, wait"));

	QGridLayout *layout = new QGridLayout(this);
	layout->addWidget(new QLabel(tr("Group")), 0, 0);
	layout->addWidget(groupList, 0, 1, 1, 3);
	layout->addWidget(new QLabel(tr("Script")), 1, 0);
	layout->addWidget(scriptList, 1, 1, 1, 3);
	layout->addWidget(new QLabel(tr("Priority")), 2, 0);
	layout->addWidget(priority, 2, 1, 1, 3);
	layout->addWidget(new QLabel(tr("Type")), 3, 0);
	layout->addWidget(execType, 3, 1, 1, 3);
	layout->addWidget(helpWidget, 4, 0, 1, 4);
	layout->setRowStretch(5, 1);
	layout->setContentsMargins(QMargins());

	connect(groupList, &QComboBox::currentIndexChanged, this, &ScriptEditorExecPage::opcodeChanged);
	connect(scriptList, &QComboBox::currentIndexChanged, this, &ScriptEditorExecPage::opcodeChanged);
	connect(groupList, &QComboBox::currentIndexChanged, this, &ScriptEditorExecPage::updateScriptList);
	connect(execType, &QComboBox::currentIndexChanged, this, &ScriptEditorExecPage::changeCurrentOpcode);
}

Opcode ScriptEditorExecPage::buildOpcode()
{
	opcode().setGroupID(quint8(groupList->currentIndex()));
	opcode().setScriptIDAndPriority(quint8(scriptList->currentIndex()), quint8(priority->value()));

	return opcode();
}

void ScriptEditorExecPage::setOpcode(const Opcode &opcode)
{
	ScriptEditorView::setOpcode(opcode);

	for (QObject *o : children()) {
		o->blockSignals(true);
	}

	switch (opcode.id()) {
	case OpcodeKey::REQ:
		execType->setCurrentIndex(0);
		break;
	case OpcodeKey::REQSW:
		execType->setCurrentIndex(1);
		break;
	case OpcodeKey::REQEW:
		execType->setCurrentIndex(2);
		break;
	default:
		break;
	}

	quint16 groupID = quint16(opcode.groupID());
	quint8 scriptID = quint8(opcode.scriptID());
	quint8 p = quint8(opcode.priority());
	
	if (groupID >= this->scriptsAndTexts()->grpScriptCount()) {
		setValid(false);

		return;
	}

	const GrpScript &grp = this->scriptsAndTexts()->grpScript(groupID);

	setValid(true);

	groupList->setCurrentIndex(groupID);

	for (quint8 i = 0; i < 32; ++i) {
		scriptList->setItemText(i, grp.scriptName(i + 1));
	}
	scriptList->setCurrentIndex(scriptID);

	priority->setValue(std::min(quint8(6), p));

	for (QObject *o : children()) {
		o->blockSignals(false);
	}
}

void ScriptEditorExecPage::updateScriptList(int groupID)
{
	if (groupID < 0 || groupID >= scriptsAndTexts()->grpScriptCount()) {
		return;
	}

	const GrpScript &grp = scriptsAndTexts()->grpScript(groupID);

	for (quint8 i = 0; i < 32; ++i) {
		scriptList->setItemText(i, grp.scriptName(i + 1));
	}
}

void ScriptEditorExecPage::changeCurrentOpcode(int index)
{
	OpcodeKey key = opcode().id();

	switch (index) {
	case 0:
		key = OpcodeKey::REQ;
		break;
	case 1:
		key = OpcodeKey::REQSW;
		break;
	case 2:
		key = OpcodeKey::REQEW;
		break;
	}

	if (key != opcode().id()) {
		opcode().op().id = key;

		emit opcodeChanged();
	}
}

ScriptEditorExecCharPage::ScriptEditorExecCharPage(const Section1File *scriptsAndTexts, const GrpScript &grpScript, const Script &script, int opcodeID, QWidget *parent) :
	ScriptEditorWithPriorityPage(scriptsAndTexts, grpScript, script, opcodeID, parent)
{
}

void ScriptEditorExecCharPage::build()
{
	ScriptEditorWithPriorityPage::build();

	partyID = new QSpinBox(this);
	partyID->setRange(0, 255);
	scriptList = new QComboBox(this);
	for (quint8 i = 0; i < 32; ++i) {
		scriptList->addItem(tr("Script %1").arg(i));
	}

	execType = new QComboBox(this);
	execType->addItem(tr("Asynchronous, no wait"));
	execType->addItem(tr("Asynchronous, wait"));
	execType->addItem(tr("Synchronous, wait"));

	QGridLayout *layout = new QGridLayout(this);
	layout->addWidget(new QLabel(tr("Team member")), 0, 0);
	layout->addWidget(partyID, 0, 1, 1, 3);
	layout->addWidget(new QLabel(tr("Script")), 1, 0);
	layout->addWidget(scriptList, 1, 1, 1, 3);
	layout->addWidget(new QLabel(tr("Priority")), 2, 0);
	layout->addWidget(priority, 2, 1, 1, 3);
	layout->addWidget(new QLabel(tr("Type")), 3, 0);
	layout->addWidget(execType, 3, 1, 1, 3);
	layout->addWidget(helpWidget, 4, 0, 1, 4);
	layout->setRowStretch(5, 1);
	layout->setContentsMargins(QMargins());

	connect(partyID, &QSpinBox::valueChanged, this, &ScriptEditorExecCharPage::opcodeChanged);
	connect(scriptList, &QComboBox::currentIndexChanged, this, &ScriptEditorExecCharPage::opcodeChanged);
	connect(execType, &QComboBox::currentIndexChanged, this, &ScriptEditorExecCharPage::changeCurrentOpcode);
}

Opcode ScriptEditorExecCharPage::buildOpcode()
{
	opcode().setPartyID(quint8(partyID->value()));
	opcode().setScriptIDAndPriority(quint8(scriptList->currentIndex()), quint8(priority->value()));

	return opcode();
}

void ScriptEditorExecCharPage::setOpcode(const Opcode &opcode)
{
	ScriptEditorView::setOpcode(opcode);

	for (QObject *o : children()) {
		o->blockSignals(true);
	}

	switch (opcode.id()) {
	case OpcodeKey::PREQ:
		execType->setCurrentIndex(0);
		break;
	case OpcodeKey::PRQSW:
		execType->setCurrentIndex(1);
		break;
	case OpcodeKey::PRQEW:
		execType->setCurrentIndex(2);
		break;
	default:
		break;
	}

	partyID->setValue(opcode.partyID());
	scriptList->setCurrentIndex(opcode.scriptID());
	priority->setValue(qMin(quint8(6), quint8(opcode.priority())));

	for (QObject *o : children()) {
		o->blockSignals(false);
	}
}

void ScriptEditorExecCharPage::changeCurrentOpcode(int index)
{
	OpcodeKey key = opcode().id();

	switch (index) {
	case 0:
		key = OpcodeKey::PREQ;
		break;
	case 1:
		key = OpcodeKey::PRQSW;
		break;
	case 2:
		key = OpcodeKey::PRQEW;
		break;
	}

	if (key != opcode().id()) {
		opcode().op().id = key;

		emit opcodeChanged();
	}
}

ScriptEditorLabelPage::ScriptEditorLabelPage(const Section1File *scriptsAndTexts, const GrpScript &grpScript, const Script &script, int opcodeID, QWidget *parent) :
	ScriptEditorView(scriptsAndTexts, grpScript, script, opcodeID, parent)
{
}

void ScriptEditorLabelPage::build()
{
	label = new QDoubleSpinBox(this);
	label->setDecimals(0);
	label->setRange(0, pow(2, 32)-1);

	QGridLayout *layout = new QGridLayout(this);
	layout->addWidget(new QLabel(tr("Label")), 0, 0);
	layout->addWidget(label, 0, 1, 1, 3);
	layout->setRowStretch(1, 1);
	layout->setContentsMargins(QMargins());

	connect(label, &QDoubleSpinBox::valueChanged, this, &ScriptEditorLabelPage::opcodeChanged);
}

Opcode ScriptEditorLabelPage::buildOpcode()
{
	opcode().setLabel(quint16(label->value()));

	return opcode();
}

void ScriptEditorLabelPage::setOpcode(const Opcode &opcode)
{
	ScriptEditorView::setOpcode(opcode);

	quint16 lbl = quint16(opcode.label());
	if (lbl == 0) {
		quint16 greaterLabel = 1;
		for (const Opcode &op : script().opcodes()) {
			if (op.id() == OpcodeKey::LABEL) {
				quint16 lbl = quint16(op.label());
				if (lbl >= greaterLabel) {
					greaterLabel = lbl + 1;
				}
			}
		}

		label->setValue(double(greaterLabel));
	} else {
		label->blockSignals(true);
		label->setValue(double(lbl));
		label->blockSignals(false);
	}
}

void ScriptEditorJumpPageInterface::fillLabelList(bool jumpBack)
{
	label->blockSignals(true);
	label->clear();

	quint32 greaterLabel = 1;
	int i = 0;

	for (const Opcode &op : script().opcodes()) {
		if (op.id() == OpcodeKey::LABEL) {
			quint16 lbl = quint16(op.label());
			if (jumpBack || i >= opcodeID()) {
				label->addItem(tr("Label %1").arg(lbl), lbl);
			}
			if (lbl >= greaterLabel) {
				greaterLabel = lbl + 1;
			}
		}

		++i;
	}

	label->addItem(tr("New label"), greaterLabel);

	int index = label->findData(opcode().label());
	if (index < 0) {
		index = 0;
	}
	// _valid = label->count() > 0;
	label->setCurrentIndex(index);
	label->blockSignals(false);
}

ScriptEditorJumpPage::ScriptEditorJumpPage(const Section1File *scriptsAndTexts, const GrpScript &grpScript, const Script &script, int opcodeID, QWidget *parent) :
	ScriptEditorJumpPageInterface(scriptsAndTexts, grpScript, script, opcodeID, parent), addJump(false)
{
}

void ScriptEditorJumpPage::build()
{
	label = new QComboBox(this);

	QGridLayout *layout = new QGridLayout(this);
	layout->addWidget(new QLabel(tr("Label")), 0, 0);
	layout->addWidget(label, 0, 1, 1, 3);
	layout->setRowStretch(1, 1);
	layout->setContentsMargins(QMargins());

	connect(label, &QComboBox::currentIndexChanged, this, &ScriptEditorJumpPage::opcodeChanged);
}

void ScriptEditorJumpPage::clear()
{
//	if (addJump) {
//		qDebug() << "ScriptEditorJumpPage deleteOpcode" << (_opcodeID+1) << _script->opcode(_opcodeID+1)->name();
//		_script->delOpcode(_opcodeID+1);
//		addJump = false;
//	}
}

Opcode ScriptEditorJumpPage::buildOpcode()
{
	opcode().setLabel(quint16(label->itemData(label->currentIndex()).toUInt()));
	opcode().setBadJump(BadJumpError::Ok);

	return opcode();
}

void ScriptEditorJumpPage::setOpcode(const Opcode &opcode)
{
	ScriptEditorView::setOpcode(opcode);

	for (QObject *o : children()) {
		o->blockSignals(true);
	}

	addJump = false;

	fillLabelList(true);

	for (QObject *o : children()) {
		o->blockSignals(false);
	}

	emit opcodeChanged();
}

bool ScriptEditorJumpPage::needsLabel() const
{
	return label->currentIndex() == label->count() - 1;
}

ScriptEditorJumpNanakiPage::ScriptEditorJumpNanakiPage(const Section1File *scriptsAndTexts, const GrpScript &grpScript, const Script &script, int opcodeID, QWidget *parent) :
    ScriptEditorJumpPageInterface(scriptsAndTexts, grpScript, script, opcodeID, parent)
{
}

void ScriptEditorJumpNanakiPage::build()
{
	label = new QComboBox(this);

	QGridLayout *layout = new QGridLayout(this);
	layout->addWidget(new QLabel(tr("Label")), 0, 0);
	layout->addWidget(label, 0, 1);
	layout->setRowStretch(1, 1);
	layout->setContentsMargins(QMargins());

	connect(label, &QComboBox::currentIndexChanged, this, &ScriptEditorJumpNanakiPage::opcodeChanged);
}

Opcode ScriptEditorJumpNanakiPage::buildOpcode()
{
	opcode().setLabel(quint16(label->itemData(label->currentIndex()).toUInt()));

	return opcode();
}

void ScriptEditorJumpNanakiPage::setOpcode(const Opcode &opcode)
{
	ScriptEditorView::setOpcode(opcode);

	for (QObject *o : children()) {
		o->blockSignals(true);
	}

	fillLabelList();

	for (QObject *o : children()) {
		o->blockSignals(false);
	}
}

bool ScriptEditorJumpNanakiPage::needsLabel() const
{
	return label->currentIndex() == label->count() - 1;
}

ScriptEditorIfPage::ScriptEditorIfPage(const Section1File *scriptsAndTexts, const GrpScript &grpScript, const Script &script, int opcodeID, QWidget *parent) :
	ScriptEditorJumpPageInterface(scriptsAndTexts, grpScript, script, opcodeID, parent), addJump(false)
{
}

void ScriptEditorIfPage::build()
{
	varOrValue1 = new VarOrValueWidget(this);
	varOrValue2 = new VarOrValueWidget(this);

	operatorList = new QComboBox(this);
	for (quint8 i = 0; i < OPERATORS_SIZE; ++i) {
		operatorList->addItem(QString::fromUtf8(Opcode::operators[i]));
	}

	rangeTest = new QComboBox();
	rangeTest->addItem(tr("8-bit unsigned"));
	rangeTest->addItem(tr("16-bit signed"));
	rangeTest->addItem(tr("16-bit unsigned"));

	label = new QComboBox(this);

	QGridLayout *layout = new QGridLayout(this);
	layout->addWidget(new QLabel(tr("Test")), 0, 0);
	layout->addWidget(varOrValue1, 0, 1);
	layout->addWidget(operatorList, 0, 2);
	layout->addWidget(varOrValue2, 0, 3);
	layout->addWidget(new QLabel(tr("Compare type")), 1, 0);
	layout->addWidget(rangeTest, 1, 1, 1, 3);
	layout->addWidget(new QLabel(tr("Label")), 2, 0);
	layout->addWidget(label, 2, 1, 1, 3);
	layout->setRowStretch(3, 1);
	layout->setContentsMargins(QMargins());

	connect(varOrValue1, &VarOrValueWidget::changed, this, &ScriptEditorIfPage::opcodeChanged);
	connect(varOrValue2, &VarOrValueWidget::changed, this, &ScriptEditorIfPage::opcodeChanged);
	connect(operatorList, &QComboBox::currentIndexChanged, this, &ScriptEditorIfPage::opcodeChanged);
	connect(label, &QComboBox::currentIndexChanged, this, &ScriptEditorIfPage::opcodeChanged);
	connect(rangeTest, &QComboBox::currentIndexChanged, this, &ScriptEditorIfPage::updateOpcode);
}

Opcode ScriptEditorIfPage::buildOpcode()
{
	FF7If i = FF7If();

	if (rangeTest->currentIndex() == 0) { // byte test
		i.size = FF7Var::Byte;
	} else if (rangeTest->currentIndex() == 1) { // signed word test
		i.size = FF7Var::SignedWord;
	} else { // unsigned word test
		i.size = FF7Var::Word;
	}

	quint8 bank1, bank2;
	int value1, value2;

	if (varOrValue1->isValue()) {
		bank1 = 0;
		value1 = varOrValue1->value();
	} else {
		quint8 address1;
		varOrValue1->var(bank1, address1);
		value1 = address1;
	}
	if (varOrValue2->isValue()) {
		bank2 = 0;
		value2 = varOrValue2->value();
	} else {
		quint8 address2;
		varOrValue2->var(bank2, address2);
		value2 = address2;
	}

	i.bank1 = bank1;
	i.bank2 = bank2;
	i.value1 = value1;
	i.value2 = value2;
	i.oper = quint8(operatorList->currentIndex());

	opcode().setIfStruct(i);
	opcode().setLabel(quint16(label->itemData(label->currentIndex()).toUInt()));

	return opcode();
}

void ScriptEditorIfPage::setOpcode(const Opcode &opcode)
{
	ScriptEditorView::setOpcode(opcode);

	addJump = false;
	
	FF7If i = FF7If();
	opcode.ifStruct(i);
	bool isLongValue = i.size != FF7Var::Byte,
	        isSignedValue = i.size == FF7Var::SignedWord;

	varOrValue1->blockSignals(true);
	varOrValue2->blockSignals(true);
	if (isLongValue) {
		varOrValue1->setLong();
	} else {
		varOrValue1->setShort();
	}
	if (isLongValue) {
		varOrValue2->setLong();
	} else {
		varOrValue2->setShort();
	}
	varOrValue1->setSignedValueType(isSignedValue);
	varOrValue2->setSignedValueType(isSignedValue);
	varOrValue1->setVarOrValue(i.bank1, i.value1);
	varOrValue2->setVarOrValue(i.bank2, i.value2);
	varOrValue1->blockSignals(false);
	varOrValue2->blockSignals(false);

	operatorList->blockSignals(true);
	operatorList->setCurrentIndex(i.oper);
	operatorList->blockSignals(false);

	fillLabelList();

	rangeTest->blockSignals(true);
	if (i.size == FF7Var::Byte) {
		rangeTest->setCurrentIndex(0);
	} else if (i.size == FF7Var::SignedWord) {
		rangeTest->setCurrentIndex(1);
	} else {
		rangeTest->setCurrentIndex(2);
	}
	rangeTest->blockSignals(false);
}

bool ScriptEditorIfPage::needsLabel() const
{
	return label->currentIndex() == label->count() - 1;
}

void ScriptEditorIfPage::updateOpcode()
{
	setOpcode(buildOpcode());

	emit opcodeChanged();
}

ScriptEditorIfKeyPage::ScriptEditorIfKeyPage(const Section1File *scriptsAndTexts, const GrpScript &grpScript, const Script &script, int opcodeID, QWidget *parent) :
	ScriptEditorJumpPageInterface(scriptsAndTexts, grpScript, script, opcodeID, parent)
{
}

void ScriptEditorIfKeyPage::build()
{
	QGridLayout *layout = new QGridLayout(this);
	layout->addWidget(new QLabel(tr("Keys")), 0, 0, 1, 4);

	int row=1, column=0;

	for (const QString &keyName : std::as_const(Data::key_names)) {
		QCheckBox *key = new QCheckBox(keyName, this);
		keys.append(key);
		connect(key, &QCheckBox::toggled, this, &ScriptEditorIfKeyPage::opcodeChanged);
		layout->addWidget(key, row, column);

		if (column == 3) {
			column = 0;
			++row;
		} else {
			++column;
		}
	}

	if (column != 0) {
		column = 0;
		++row;
	}

	label = new QComboBox(this);
	typeList = new QComboBox(this);
	typeList->addItem(tr("Key pressed"));
	typeList->addItem(tr("Key pressed once"));
	typeList->addItem(tr("Key released once"));

	layout->addWidget(new QLabel(tr("Type")), row, column);
	layout->addWidget(typeList, row, column+1, 1, 3);
	layout->addWidget(new QLabel(tr("Label")), row+1, column);
	layout->addWidget(label, row+1, column+1, 1, 3);
	layout->setRowStretch(row+2, 1);
	layout->setContentsMargins(QMargins());

	connect(label, &QComboBox::currentIndexChanged, this, &ScriptEditorIfKeyPage::opcodeChanged);
	connect(typeList, &QComboBox::currentIndexChanged, this, &ScriptEditorIfKeyPage::opcodeChanged);
}

Opcode ScriptEditorIfKeyPage::buildOpcode()
{
	if (typeList->currentIndex() == 0) {
		opcode().op().id = OpcodeKey::IFKEY;
	} else if (typeList->currentIndex() == 1) {
		opcode().op().id = OpcodeKey::IFKEYON;
	} else {
		opcode().op().id = OpcodeKey::IFKEYOFF;
	}

	quint16 result = 0;

	for (int i = 0; i < keys.size(); ++i) {
		if (keys.at(i)->isChecked()) {
			result |= 1 << i;
		}
	}

	opcode().setKeys(result);
	opcode().setLabel(quint16(label->itemData(label->currentIndex()).toUInt()));

	return opcode();
}

void ScriptEditorIfKeyPage::setOpcode(const Opcode &opcode)
{
	ScriptEditorView::setOpcode(opcode);

	for (QObject *o : children()) {
		o->blockSignals(true);
	}

	switch (opcode.id()) {
	case OpcodeKey::IFKEY:
		typeList->setCurrentIndex(0);
		break;
	case OpcodeKey::IFKEYON:
		typeList->setCurrentIndex(1);
		break;
	case OpcodeKey::IFKEYOFF:
		typeList->setCurrentIndex(2);
		break;
	default: break;
	}

	quint16 ks = quint16(opcode.keys());

	for (int i = 0; i < keys.size(); ++i) {
		keys.at(i)->setChecked(bool((ks >> i) & 1));
	}

	fillLabelList();

	for (QObject *o : children()) {
		o->blockSignals(false);
	}
}

bool ScriptEditorIfKeyPage::needsLabel() const
{
	return label->currentIndex() == label->count() - 1;
}

ScriptEditorIfQPage::ScriptEditorIfQPage(const Section1File *scriptsAndTexts, const GrpScript &grpScript, const Script &script, int opcodeID, QWidget *parent) :
	ScriptEditorJumpPageInterface(scriptsAndTexts, grpScript, script, opcodeID, parent)
{
}

void ScriptEditorIfQPage::build()
{
	charList = new QComboBox(this);
	charList->addItems(Data::char_names);
	int nbItems = charList->count();
	for (int i = nbItems; i < 100; i++) {
		charList->addItem(QString("%1").arg(i));
	}
	for (quint8 i = 100; i < 254; i++) {
		charList->addItem(Data::char_names.last());
	}

	charList->addItem(tr("(Empty)"));
	charList->addItem(tr("(Empty)"));

	label = new QComboBox(this);

	QGridLayout *layout = new QGridLayout(this);
	layout->addWidget(new QLabel(tr("Character")), 0, 0);
	layout->addWidget(charList, 0, 1, 1, 3);
	layout->addWidget(new QLabel(tr("Label")), 1, 0);
	layout->addWidget(label, 1, 1, 1, 3);
	layout->setRowStretch(2, 1);
	layout->setContentsMargins(QMargins());

	connect(charList, &QComboBox::currentIndexChanged, this, &ScriptEditorIfQPage::opcodeChanged);
	connect(label, &QComboBox::currentIndexChanged, this, &ScriptEditorIfQPage::opcodeChanged);
}

Opcode ScriptEditorIfQPage::buildOpcode()
{
	opcode().setCharID(quint8(charList->currentIndex()));
	opcode().setLabel(quint16(label->itemData(label->currentIndex()).toUInt()));

	return opcode();
}

void ScriptEditorIfQPage::setOpcode(const Opcode &opcode)
{
	ScriptEditorView::setOpcode(opcode);

	for (QObject *o : children()) {
		o->blockSignals(true);
	}

	charList->setCurrentIndex(opcode.charID());

	fillLabelList();

	for (QObject *o : children()) {
		o->blockSignals(false);
	}
}

bool ScriptEditorIfQPage::needsLabel() const
{
	return label->currentIndex() == label->count() - 1;
}

ScriptEditorWaitPage::ScriptEditorWaitPage(const Section1File *scriptsAndTexts, const GrpScript &grpScript, const Script &script, int opcodeID, QWidget *parent) :
	ScriptEditorView(scriptsAndTexts, grpScript, script, opcodeID, parent)
{
}

void ScriptEditorWaitPage::build()
{
	frames = new QSpinBox(this);
	frames->setRange(0, 65535);

	QGridLayout *layout = new QGridLayout(this);
	layout->addWidget(new QLabel(tr("Images")), 0, 0);
	layout->addWidget(frames, 0, 1);
	layout->setRowStretch(1, 1);
	layout->setContentsMargins(QMargins());

	connect(frames, &QSpinBox::valueChanged, this, &ScriptEditorWaitPage::opcodeChanged);
}

Opcode ScriptEditorWaitPage::buildOpcode()
{
	OpcodeWAIT &opcodeWAIT = opcode().op().opcodeWAIT;
	opcodeWAIT.frameCount = quint16(frames->value());

	return opcode();
}

void ScriptEditorWaitPage::setOpcode(const Opcode &opcode)
{
	ScriptEditorView::setOpcode(opcode);

	for (QObject *o : children()) {
		o->blockSignals(true);
	}

	const OpcodeWAIT &opcodeWAIT = opcode.op().opcodeWAIT;
	frames->setValue(opcodeWAIT.frameCount);

	for (QObject *o : children()) {
		o->blockSignals(false);
	}
}

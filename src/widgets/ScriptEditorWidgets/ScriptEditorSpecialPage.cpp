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
#include "ScriptEditorSpecialPage.h"

ScriptEditorSpecialPName::ScriptEditorSpecialPName(const Section1File *scriptsAndTexts, const GrpScript &grpScript, const Script &script,
                                                   int opcodeID, QWidget *parent) :
    ScriptEditorView(scriptsAndTexts, grpScript, script, opcodeID, parent)
{
}

void ScriptEditorSpecialPName::build()
{
	_varOrValue = new VarOrValueWidget(this);
	
	_bank = new VarOrValueWidget(this);
	_bank->setOnlyBank(true);
	_bank->setShort();

	_size = new QSpinBox(this);
	_size->setRange(0, 255);

	QGridLayout *layout = new QGridLayout(this);
	layout->addWidget(new QLabel(tr("Var or value")), 0, 0);
	layout->addWidget(_varOrValue, 0, 1);
	layout->addWidget(new QLabel(tr("Bank")), 1, 0);
	layout->addWidget(_bank, 1, 1);
	layout->addWidget(new QLabel(tr("Size")), 2, 0);
	layout->addWidget(_size, 2, 1);
	layout->setRowStretch(3, 1);
	layout->setColumnStretch(2, 1);
	layout->setContentsMargins(QMargins());

	connect(_varOrValue, SIGNAL(changed()), SIGNAL(opcodeChanged()));
	connect(_bank, SIGNAL(changed()), SIGNAL(opcodeChanged()));
	connect(_size, SIGNAL(editingFinished()), SIGNAL(opcodeChanged()));
}

Opcode ScriptEditorSpecialPName::buildOpcode()
{
	OpcodeSPECIALPNAME &opcodeSpecialPname = opcode().op().opcodeSPECIALPNAME;
	
	quint8 bank1 = 0;
	int value;

	if (_varOrValue->isValue()) {
		value = _varOrValue->value();
	} else {
		quint8 address1;
		_varOrValue->var(bank1, address1);
		value = address1;
	}

	opcodeSpecialPname.banks = BANK(bank1, _bank->bank());
	opcodeSpecialPname.varOrValue = quint8(value);
	opcodeSpecialPname.size = quint8(_size->value());

	return opcode();
}

void ScriptEditorSpecialPName::setOpcode(const Opcode &opcode)
{
	const OpcodeSPECIALPNAME &opcodeSpecialPname = opcode.op().opcodeSPECIALPNAME;

	_varOrValue->setVarOrValue(B1(opcodeSpecialPname.banks), opcodeSpecialPname.varOrValue);
	_bank->setBank(B2(opcodeSpecialPname.banks));
	_size->setValue(opcodeSpecialPname.size);

	ScriptEditorView::setOpcode(opcode);
}

ScriptEditorDLPBSavemap::ScriptEditorDLPBSavemap(const Section1File *scriptsAndTexts, const GrpScript &grpScript, const Script &script,
                                                 int opcodeID, QWidget *parent) :
    ScriptEditorView(scriptsAndTexts, grpScript, script, opcodeID, parent)
{
}

void ScriptEditorDLPBSavemap::build()
{
	_from = new QSpinBox(this);
	_from->setDisplayIntegerBase(16);
	_from->setRange(0, 65535);
	_to = new QSpinBox(this);
	_to->setDisplayIntegerBase(16);
	_to->setRange(0, 65535);
	_absValue = new QDoubleSpinBox(this);
	_absValue->setDecimals(0);
	_absValue->setRange(qint32(0x80000000), pow(2, 32)-1);

	_flag = new QComboBox(this);
	_flag->addItem(tr("8 bit"), 0);
	_flag->addItem(tr("16 bit"), 1);
	_flag->addItem(tr("24 bit"), 2);
	_flag->addItem(tr("32 bit"), 3);

	_fromIsPointer = new QCheckBox(tr("From is a pointer"), this);
	_toIsPointer = new QCheckBox(tr("To is a pointer"), this);

	QGridLayout *layout = new QGridLayout(this);
	layout->addWidget(new QLabel(tr("From")), 0, 0);
	layout->addWidget(_from, 0, 1, 1, 3);
	layout->addWidget(new QLabel(tr("To")), 1, 0);
	layout->addWidget(_to, 1, 1, 1, 3);
	layout->addWidget(new QLabel(tr("Abs Value")), 2, 0);
	layout->addWidget(_absValue, 2, 1, 1, 3);
	layout->addWidget(new QLabel(tr("Size")), 3, 0);
	layout->addWidget(_flag, 3, 1, 1, 3);
	layout->addWidget(_fromIsPointer, 4, 0, 1, 2);
	layout->addWidget(_toIsPointer, 4, 2, 1, 2);
	layout->setRowStretch(5, 1);
	layout->setColumnStretch(2, 1);
	layout->setContentsMargins(QMargins());
	
	connect(_from, &QSpinBox::valueChanged, this, &ScriptEditorDLPBSavemap::setAbsValueEnabled);
	connect(_from, &QSpinBox::editingFinished, this, &ScriptEditorDLPBSavemap::opcodeChanged);
	connect(_to, &QSpinBox::editingFinished, this, &ScriptEditorDLPBSavemap::opcodeChanged);
	connect(_absValue, &QDoubleSpinBox::editingFinished, this, &ScriptEditorDLPBSavemap::opcodeChanged);
	connect(_flag, &QComboBox::currentIndexChanged, this, &ScriptEditorDLPBSavemap::opcodeChanged);
	connect(_fromIsPointer, &QCheckBox::toggled, this, &ScriptEditorDLPBSavemap::opcodeChanged);
	connect(_toIsPointer, &QCheckBox::toggled, this, &ScriptEditorDLPBSavemap::opcodeChanged);
}

Opcode ScriptEditorDLPBSavemap::buildOpcode()
{
	OpcodeUnused1A &op = opcode().op().opcodeUnused1A;

	op.from = quint16(_from->value());
	op.to = quint16(_to->value());
	op.absValue = quint32(_absValue->value());
	op.flag = quint8((_toIsPointer->isChecked() << 5) |
	                 (_fromIsPointer->isChecked() << 4) |
	                 quint8(_flag->currentData().toInt()));

	return opcode();
}

void ScriptEditorDLPBSavemap::setOpcode(const Opcode &opcode)
{
	ScriptEditorView::setOpcode(opcode);

	const OpcodeUnused1A &o = opcode.op().opcodeUnused1A;

	_from->setValue(o.from);
	_to->setValue(o.to);
	_absValue->setValue(o.absValue);
	_flag->setCurrentIndex(_flag->findData(o.flag & 0x7));
	_fromIsPointer->setChecked(o.flag & 0x10);
	_toIsPointer->setChecked(o.flag & 0x20);
}

void ScriptEditorDLPBSavemap::setAbsValueEnabled(int value)
{
	_absValue->setEnabled(value == 0xFFFF);
}

ScriptEditorDLPBWriteToMemory::ScriptEditorDLPBWriteToMemory(const Section1File *scriptsAndTexts, const GrpScript &grpScript, const Script &script,
                                                             int opcodeID, QWidget *parent) :
      ScriptEditorView(scriptsAndTexts, grpScript, script, opcodeID, parent)
{
}

void ScriptEditorDLPBWriteToMemory::build()
{
	_address = new QSpinBox(this);
	_address->setDisplayIntegerBase(16);
	_address->setRange(0, 0x7FFFFFFF);
	_bytes = new QLineEdit(this);

	QGridLayout *layout = new QGridLayout(this);
	layout->addWidget(new QLabel(tr("Address")), 0, 0);
	layout->addWidget(_address, 0, 1, 1, 3);
	layout->addWidget(new QLabel(tr("Bytes")), 1, 0);
	layout->addWidget(_bytes, 1, 1, 1, 3);
	layout->setRowStretch(2, 1);
	layout->setContentsMargins(QMargins());

	connect(_address, &QSpinBox::editingFinished, this, &ScriptEditorDLPBWriteToMemory::opcodeChanged);
	connect(_bytes, &QLineEdit::editingFinished, this, &ScriptEditorDLPBWriteToMemory::opcodeChanged);
}

Opcode ScriptEditorDLPBWriteToMemory::buildOpcode()
{
	OpcodeUnused1C &op = opcode().op().opcodeUnused1C;
	QByteArray data = QByteArray::fromHex(_bytes->text().toLatin1()).left(128);

	op.address = quint32(_address->value());
	opcode().setResizableData(data);
	op.subSize = data.size();

	return opcode();
}

void ScriptEditorDLPBWriteToMemory::setOpcode(const Opcode &opcode)
{
	ScriptEditorView::setOpcode(opcode);

	const OpcodeUnused1C &op = opcode.op().opcodeUnused1C;

	_address->setValue(op.address);
	_bytes->setText(QString::fromLatin1(opcode.resizableData().toHex()));
}

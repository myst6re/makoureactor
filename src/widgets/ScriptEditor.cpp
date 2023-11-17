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
#include "ScriptEditor.h"
#include "ScriptEditorWidgets/ScriptEditorGenericList.h"
#include "ScriptEditorWidgets/ScriptEditorStructPage.h"
#include "ScriptEditorWidgets/ScriptEditorMathPage.h"
#include "ScriptEditorWidgets/ScriptEditorWindowPage.h"
#include "ScriptEditorWidgets/ScriptEditorModelPage.h"
#include "ScriptEditorWidgets/ScriptEditorMoviePage.h"
#include "ScriptEditorWidgets/ScriptEditorSpecialPage.h"
#include "ScriptEditorWidgets/ScriptEditorWalkmeshPage.h"

QList<OpcodeKey> ScriptEditor::crashIfInit;

ScriptEditor::ScriptEditor(Field *field, const Section1File *scriptsAndTexts, const GrpScript &grpScript, const Script &script, int opcodeID, bool modify, bool isInit, QWidget *parent) :
	QDialog(parent, Qt::Dialog | Qt::WindowCloseButtonHint), _currentPageWidget(nullptr), _currentPageType(GenericList),
	field(field), scriptsAndTexts(scriptsAndTexts), grpScript(grpScript), script(script), opcodeID(opcodeID), isInit(isInit), modify(modify), change(false)
{
	if (crashIfInit.isEmpty()) {
		crashIfInit << OpcodeKey::JOIN << OpcodeKey::SPLIT
		            << OpcodeKey::DSKCG
		            << OpcodeKey::MINIGAME << OpcodeKey::TUTOR
		            << OpcodeKey::PMOVA
		            << OpcodeKey::PTURA
		            << OpcodeKey::MESSAGE << OpcodeKey::ASK << OpcodeKey::MENU
		            << OpcodeKey::WINDOW << OpcodeKey::MAPJUMP
		            << OpcodeKey::DFANM << OpcodeKey::ANIME1
		            << OpcodeKey::MOVE << OpcodeKey::CMOVE << OpcodeKey::FMOVE
		            << OpcodeKey::ANIME2 << OpcodeKey::ANIMX1 << OpcodeKey::CANIM1 << OpcodeKey::CANMX1
		            << OpcodeKey::TURNGEN << OpcodeKey::TURN << OpcodeKey::ANIMX2 << OpcodeKey::CANIM2
		            << OpcodeKey::CANMX2 << OpcodeKey::JUMP << OpcodeKey::LADER;
	}

	setWindowTitle(tr("Script Editor%1").arg(isInit ? tr(" (init mode)") : ""));
	setMinimumSize(640, 480);
	
	QStringList liste0;
	liste0 << tr("Control Structures") <<
	          tr("Mathematics") <<
	          tr("Windowing and messages") <<
	          tr("Party and inventory") <<
	          tr("Field Models and animations") <<
	          tr("Walkmesh and locations") <<
	          tr("Background") <<
	          tr("Fade and camera") <<
	          tr("Audio and video") <<
	          tr("Modules") <<
	          tr("Unknown");
	
	comboBox0 = new QComboBox(this);
	comboBox0->addItems(liste0);
	comboBox = new QComboBox(this);
	buildList(0);
	
	textEdit = new QPlainTextEdit(this);
	textEdit->setReadOnly(true);
	// 2 Lines
	const int textEditHMargins = textEdit->contentsMargins().top() + textEdit->contentsMargins().bottom()
	                             + int(textEdit->document()->documentMargin() * 2.0);
	textEdit->setFixedHeight(2 * textEdit->fontMetrics().height() + textEditHMargins);

	editorLayout = new QStackedWidget(this);

	ok = new QPushButton(tr("OK"),this);
	ok->setDefault(true);
	QPushButton *cancel = new QPushButton(tr("Cancel"),this);
	
	QHBoxLayout *buttonLayout = new QHBoxLayout();
	buttonLayout->addStretch();
	buttonLayout->addWidget(ok);
	buttonLayout->addWidget(cancel);
	
	QVBoxLayout *layout = new QVBoxLayout(this);
	layout->addWidget(comboBox0);
	layout->addWidget(comboBox);
	layout->addWidget(textEdit);
	layout->addWidget(editorLayout);
	layout->addStretch();
	layout->addLayout(buttonLayout);

	if (modify) {
		this->opcode = script.opcode(opcodeID);
		int id = opcode.id();

		if (id == OpcodeKey::SPECIAL) {
			id = (opcode.op().opcodeSPECIAL.subKey << 8) | id;
		} else if (id == OpcodeKey::KAWAI) {
			id = (opcode.op().opcodeKAWAI.subKey << 8) | id;
		}

		setCurrentMenu(id);
		fillView();
	} else {
		this->opcode = OpcodeRET();
		comboBox->setCurrentIndex(0);
		changeCurrentOpcode(0);
	}

	connect(ok, &QPushButton::clicked, this, &ScriptEditor::accept);
	connect(cancel, &QPushButton::clicked, this, &ScriptEditor::close);
	connect(comboBox0, &QComboBox::currentIndexChanged, this, &ScriptEditor::buildList);
	connect(comboBox, &QComboBox::currentIndexChanged, this, &ScriptEditor::changeCurrentOpcode);
}

ScriptEditorView *ScriptEditor::buildEditorPage(PageType id)
{
	switch (id) {
	case GenericList:
		return new ScriptEditorGenericList(scriptsAndTexts, grpScript, script, opcodeID, this);
	case NoParameters:
		return new ScriptEditorNoParameterPage(scriptsAndTexts, grpScript, script, opcodeID, this);
	case Boolean:
		return new ScriptEditorBooleanPage(scriptsAndTexts, grpScript, script, opcodeID, this);
	case OneVarOrValue:
		return new ScriptEditorOneVarOrValue(scriptsAndTexts, grpScript, script, opcodeID, this);
	case ReturnTo:
		return new ScriptEditorReturnToPage(scriptsAndTexts, grpScript, script, opcodeID, this);
	case Exec:
		return new ScriptEditorExecPage(scriptsAndTexts, grpScript, script, opcodeID, this);
	case ExecChar:
		return new ScriptEditorExecCharPage(scriptsAndTexts, grpScript, script, opcodeID, this);
	case Label:
		return new ScriptEditorLabelPage(scriptsAndTexts, grpScript, script, opcodeID, this);
	case Jump:
		return new ScriptEditorJumpPage(scriptsAndTexts, grpScript, script, opcodeID, this);
	case If:
		return new ScriptEditorIfPage(scriptsAndTexts, grpScript, script, opcodeID, this);
	case IfKey:
		return new ScriptEditorIfKeyPage(scriptsAndTexts, grpScript, script, opcodeID, this);
	case IfQ:
		return new ScriptEditorIfQPage(scriptsAndTexts, grpScript, script, opcodeID, this);
	case Wait:
		return new ScriptEditorWaitPage(scriptsAndTexts, grpScript, script, opcodeID, this);
	case BinaryOp:
		return new ScriptEditorBinaryOpPage(scriptsAndTexts, grpScript, script, opcodeID, this);
	case UnaryOp:
		return new ScriptEditorUnaryOpPage(scriptsAndTexts, grpScript, script, opcodeID, this);
	case BitOp:
		return new ScriptEditorBitOpPage(scriptsAndTexts, grpScript, script, opcodeID, this);
	case Variable:
		return new ScriptEditorVariablePage(scriptsAndTexts, grpScript, script, opcodeID, this);
	case ToByte:
		return new ScriptEditor2BytePage(scriptsAndTexts, grpScript, script, opcodeID, this);
	case SinCos:
		return new ScriptEditorSinCosPage(scriptsAndTexts, grpScript, script, opcodeID, this);
	case Window:
		return new ScriptEditorWindowPage(scriptsAndTexts, grpScript, script, opcodeID, this);
	case WindowMode:
		return new ScriptEditorWindowModePage(scriptsAndTexts, grpScript, script, opcodeID, this);
	case WindowMove:
		return new ScriptEditorWindowMovePage(scriptsAndTexts, grpScript, script, opcodeID, this);
	case WindowVar:
		return new ScriptEditorWindowVariablePage(scriptsAndTexts, grpScript, script, opcodeID, this);
	case WindowNumDisplay:
		return new ScriptEditorWindowNumDisplayPage(scriptsAndTexts, grpScript, script, opcodeID, this);
	case Model:
		return new ScriptEditorModelPage(field, scriptsAndTexts, grpScript, script, opcodeID, this);
	case Movie:
		return new ScriptEditorMoviePage(scriptsAndTexts, grpScript, script, opcodeID, this);
	case Walkmesh:
		return new ScriptEditorWalkmeshPage(field, scriptsAndTexts, grpScript, script, opcodeID, this);
	case JumpNanaki:
		return new ScriptEditorJumpNanakiPage(scriptsAndTexts, grpScript, script, opcodeID, this);
	case SpecialPName:
		return new ScriptEditorSpecialPName(scriptsAndTexts, grpScript, script, opcodeID, this);
	case DLPBSavemap:
		return new ScriptEditorDLPBSavemap(scriptsAndTexts, grpScript, script, opcodeID, this);
	case DLPBWriteToMemory:
		return new ScriptEditorDLPBWriteToMemory(scriptsAndTexts, grpScript, script, opcodeID, this);
	}
	return nullptr;
}

void ScriptEditor::fillEditor()
{
	if (_currentPageWidget != nullptr) {
		_currentPageWidget->clear();
	}

	// Change current editor widget
	switch (opcode.id()) {
	case OpcodeKey::RET:
	case OpcodeKey::KAWIW:
	case OpcodeKey::HMPMAX1:
	case OpcodeKey::HMPMAX2:
	case OpcodeKey::MHMMX:
	case OpcodeKey::HMPMAX3:
	case OpcodeKey::Unused44:
	case OpcodeKey::Unused46:
	case OpcodeKey::Unused4C:
	case OpcodeKey::Unused4E:
	case OpcodeKey::NOP:
	case OpcodeKey::SCRCC:
	case OpcodeKey::SCRLW:
	case OpcodeKey::FADEW:
	case OpcodeKey::ANIMW:
	case OpcodeKey::UnusedBE:
	case OpcodeKey::OFSTW:
	case OpcodeKey::PMJMP2:
	case OpcodeKey::ANIMB:
	case OpcodeKey::TURNW:
	case OpcodeKey::MOVIE:
	case OpcodeKey::GAMEOVER:
		_currentPageType = NoParameters;
		break;
	case OpcodeKey::REQEW:case OpcodeKey::REQ:
	case OpcodeKey::REQSW:
		_currentPageType = Exec;
		break;
	case OpcodeKey::PRQEW:case OpcodeKey::PREQ:
	case OpcodeKey::PRQSW:
		_currentPageType = ExecChar;
		break;
	case OpcodeKey::RETTO:
		_currentPageType = ReturnTo;
		break;
	case OpcodeKey::SPECIAL:
		switch (OpcodeSpecialKey(opcode.subKey())) {
		case OpcodeSpecialKey::ARROW:
		case OpcodeSpecialKey::BTLCK:
		case OpcodeSpecialKey::MVLCK:
			_currentPageType = Boolean;
			break;
		case OpcodeSpecialKey::FLMAT:
		case OpcodeSpecialKey::FLITM:
		case OpcodeSpecialKey::RSGLB:
		case OpcodeSpecialKey::CLITM:
			_currentPageType = NoParameters;
			break;
		case OpcodeSpecialKey::PNAME:
			_currentPageType = SpecialPName;
			break;
		case OpcodeSpecialKey::GMSPD:
		case OpcodeSpecialKey::SMSPD:
			_currentPageType = OneVarOrValue;
			break;
		default:
			_currentPageType = GenericList;
		}
		break;
	case OpcodeKey::JMPF:case OpcodeKey::JMPFL:
	case OpcodeKey::JMPB:case OpcodeKey::JMPBL:
		_currentPageType = Jump;
		break;
	case OpcodeKey::IFUB:case OpcodeKey::IFUBL:
	case OpcodeKey::IFSW:case OpcodeKey::IFSWL:
	case OpcodeKey::IFUW:case OpcodeKey::IFUWL:
		_currentPageType = If;
		break;
	case OpcodeKey::IFKEY:case OpcodeKey::IFKEYON:
	case OpcodeKey::IFKEYOFF:
		_currentPageType = IfKey;
		break;
	case OpcodeKey::IFMEMBQ:case OpcodeKey::IFPRTYQ:
		_currentPageType = IfQ;
		break;
	case OpcodeKey::WAIT:
		_currentPageType = Wait;
		break;
	case OpcodeKey::PLUSX:case OpcodeKey::PLUS2X:
	case OpcodeKey::MINUSX:case OpcodeKey::MINUS2X:
	case OpcodeKey::SETBYTE:case OpcodeKey::SETWORD:
	case OpcodeKey::PLUS:case OpcodeKey::PLUS2:
	case OpcodeKey::MINUS:case OpcodeKey::MINUS2:
	case OpcodeKey::MUL:case OpcodeKey::MUL2:
	case OpcodeKey::DIV:case OpcodeKey::DIV2:
	case OpcodeKey::MOD:case OpcodeKey::MOD2:
	case OpcodeKey::AND:case OpcodeKey::AND2:
	case OpcodeKey::OR:case OpcodeKey::OR2:
	case OpcodeKey::XOR:case OpcodeKey::XOR2:
	case OpcodeKey::LBYTE:case OpcodeKey::HBYTE:
		_currentPageType = BinaryOp;
		break;
	case OpcodeKey::INCX:case OpcodeKey::INC2X:
	case OpcodeKey::INC:case OpcodeKey::INC2:
	case OpcodeKey::DECX:case OpcodeKey::DEC2X:
	case OpcodeKey::DEC:case OpcodeKey::DEC2:
	case OpcodeKey::RANDOM:
		_currentPageType = UnaryOp;
		break;
	case OpcodeKey::BITOFF:case OpcodeKey::BITON:
	case OpcodeKey::BITXOR:
		_currentPageType = BitOp;
		break;
	case OpcodeKey::TOBYTE:
		_currentPageType = ToByte;
		break;
	case OpcodeKey::SIN:
	case OpcodeKey::COS:
		_currentPageType = SinCos;
		break;
	case OpcodeKey::RDMSD:
	/* case OpcodeKey::BTRLD:
	case OpcodeKey::TALKR:
	case OpcodeKey::SLIDR:
	case OpcodeKey::TLKR2:
	case OpcodeKey::SLDR2:
	case OpcodeKey::BATTLE:
	case OpcodeKey::LSTMP:
	case OpcodeKey::MVIEF:
	case OpcodeKey::CHMST: */
		_currentPageType = Variable;
		break;
	case OpcodeKey::WSPCL:
		_currentPageType = WindowNumDisplay;
		break;
	case OpcodeKey::WINDOW:case OpcodeKey::WSIZW:
	case OpcodeKey::WROW:
		_currentPageType = Window;
		break;
	case OpcodeKey::WMODE:
		_currentPageType = WindowMode;
		break;
	case OpcodeKey::WMOVE:
		_currentPageType = WindowMove;
		break;
	case OpcodeKey::MPARA:
	case OpcodeKey::MPRA2:
		_currentPageType = WindowVar;
		break;
	case OpcodeKey::CHAR_:
		_currentPageType = Model;
		break;
	case OpcodeKey::PMVIE:
		_currentPageType = Movie;
		break;
	case OpcodeKey::LINE:
		_currentPageType = Walkmesh;
		break;
	case OpcodeKey::Unused1A:
		_currentPageType = DLPBSavemap;
		break;
	case OpcodeKey::Unused1B:
		_currentPageType = JumpNanaki;
		break;
	case OpcodeKey::Unused1C:
		_currentPageType = DLPBWriteToMemory;
		break;
	case OpcodeKey::LINON:
	case OpcodeKey::MENU2:
	case OpcodeKey::VISI:
	case OpcodeKey::FCFIX:
	case OpcodeKey::BLINK:
	case OpcodeKey::TLKON:
	case OpcodeKey::SOLID:
	case OpcodeKey::SLIP:
	case OpcodeKey::BGMOVIE:
	case OpcodeKey::BTLON:
	case OpcodeKey::MPJPO:
	case OpcodeKey::UC:
		_currentPageType = Boolean;
		break;
	case OpcodeKey::LABEL:
		_currentPageType = Label;
		break;
	default:
		_currentPageType = GenericList;
	}
	
	if (!_typeToIndex.contains(_currentPageType)) {
		_currentPageWidget = buildEditorPage(_currentPageType);
		connect(_currentPageWidget, &ScriptEditorView::opcodeChanged, this, &ScriptEditor::refreshTextEdit);
		_typeToIndex.insert(_currentPageType, editorLayout->addWidget(_currentPageWidget));
	} else {
		_currentPageWidget = static_cast<ScriptEditorView *>(editorLayout->widget(_typeToIndex.value(_currentPageType)));
	}
	bool hasParams = opcode.size() > 1 || opcode.id() == OpcodeKey::LABEL;
	editorLayout->setVisible(hasParams);
	editorLayout->setCurrentWidget(_currentPageWidget);
	_currentPageWidget->setOpcode(opcode);
}

void ScriptEditor::fillView()
{
	textEdit->setPlainText(opcode.toString(scriptsAndTexts)); // text part
	fillEditor(); // editor part

	// disable if necessary
	bool disableEditor = (isInit && crashIfInit.contains(opcode.id())) || !_currentPageWidget->isValid();

	textEdit->setDisabled(disableEditor);
	_currentPageWidget->setDisabled(disableEditor);
	ok->setDisabled(disableEditor);
}

bool ScriptEditor::needslabel() const
{
	return (_currentPageType == Jump
	        || _currentPageType == If
	        || _currentPageType == IfKey
	        || _currentPageType == IfQ
	        || _currentPageType == JumpNanaki)
	        && static_cast<ScriptEditorJumpPageInterface *>(_currentPageWidget)->needsLabel();
}

Opcode ScriptEditor::buildOpcode() const
{
	return _currentPageWidget->buildOpcode();
}

void ScriptEditor::refreshTextEdit()
{
	this->change = true;

	opcode = _currentPageWidget->buildOpcode();

	textEdit->setPlainText(opcode.toString(scriptsAndTexts));
}

void ScriptEditor::changeCurrentOpcode(int index)
{
	this->change = true;

	const QList<QVariant> itemDataList = comboBox->itemData(index).toList();
	const int itemData = !itemDataList.isEmpty() ? itemDataList.first().toInt() : 0;

	// Create opcode

	if (OpcodeKey::LABEL == itemData) { // LABEL exception
		if (OpcodeKey::LABEL == opcode.id()) {
			opcode.setLabel(0);
		} else {
			OpcodeLABEL opcodeLabel;
			opcodeLabel._label = 0;
			opcode = opcodeLabel;
		}
	} else {
		const quint8 id = itemData & 0xFF;
		QByteArray newOpcode;
		newOpcode.append(char(id));

		// Fill opcode with \x00

		if (OpcodeKey::KAWAI == id) { //KAWAI
			newOpcode.append('\x03'); // size
			newOpcode.append(char((itemData >> 8) & 0xFF)); // KAWAI ID
		} else if (OpcodeKey::SPECIAL == id) { //SPECIAL
			quint8 byte2 = (itemData >> 8) & 0xFF;
			newOpcode.append(char(byte2)); // SPECIAL ID
			switch (byte2)
			{
			case 0xF5:case 0xF6:case 0xF7:case 0xFB:case 0xFC:
				newOpcode.append('\x00');
				break;
			case 0xF8:case 0xFD:
				newOpcode.append('\x00');
				newOpcode.append('\x00');
				break;
			}
		} else {
			for (quint8 pos=1; pos<Opcode::length[id]; ++pos) {
				newOpcode.append('\x00');
			}
		}

		if (id == opcode.id()) {
			opcode.setParams(newOpcode.constData() + 1, int(newOpcode.size() - 1)); // same opcode, just change params
		} else { // change all
			opcode = Opcode(newOpcode.constData(), newOpcode.size());
		}
	}

	fillView();
}

void ScriptEditor::setCurrentMenu(int id)
{
	for (int i = 0; i < comboBox0->count(); ++i) {
		buildList(i);
		int index = -1;
		for (int j = 0; j < comboBox->count() && index == -1; ++j) {
			QList<QVariant> dataList = comboBox->itemData(j).toList();
			for (const QVariant &v : std::as_const(dataList)) {
				if (v.toInt() == id) {
					index = j;
					break;
				}
			}
		}

		if (index != -1) {
			comboBox0->setCurrentIndex(i);
			comboBox->setCurrentIndex(index);
			setEnabled(true);
			return;
		}
	}

	setEnabled(false);
}

void ScriptEditor::buildList(int id)
{
	comboBox->clear();
	switch (id) {
	case 0:
		comboBox->addItem(tr("Return"), QList<QVariant>() << 0x00);
		comboBox->addItem(tr("Return to"), QList<QVariant>() << 0x07);
		comboBox->insertSeparator(comboBox->count());
		comboBox->addItem(tr("Execute a script"), QList<QVariant>()
						  << 0x01 << 0x02 << 0x03);
		comboBox->addItem(tr("Execute a script from a party member"), QList<QVariant>()
						  << 0x04 << 0x05 << 0x06);
		comboBox->insertSeparator(comboBox->count());
		comboBox->addItem(tr("Label"), QList<QVariant>() << 0x100);
		comboBox->addItem(tr("Goto label"), QList<QVariant>() << 0x10 << 0x11 << 0x12 << 0x13);
		comboBox->insertSeparator(comboBox->count());
		comboBox->addItem(tr("If...then"), QList<QVariant>()
						  << 0x14 << 0x15 << 0x16 << 0x17 << 0x18 << 0x19);
		comboBox->addItem(tr("If key pressed"), QList<QVariant>() << 0x30 << 0x31 << 0x32);
		comboBox->addItem(tr("If Party Member"), QList<QVariant>() << 0xCB);
		comboBox->addItem(tr("If character exists"), QList<QVariant>() << 0xCC);
		comboBox->addItem(tr("[DLPB's custom opcode] If Red XIII is named Nanaki"), QList<QVariant>() << 0x1B);
		comboBox->insertSeparator(comboBox->count());
		comboBox->addItem(tr("Wait"), QList<QVariant>() << 0x24);
		comboBox->addItem(tr("No Operation"), QList<QVariant>() << 0x5F);
		return;
	case 1:
		comboBox->addItem(tr("Binary operation"), QList<QVariant>()
						  << 0x80 << 0x81 << 0x76 << 0x85 << 0x77 << 0x86 << 0x78
						  << 0x87 << 0x79 << 0x88 << 0x89 << 0x8A << 0x8B << 0x8C
						  << 0x8D << 0x8E << 0x8F << 0x90 << 0x91 << 0x92 << 0x93
						  << 0x94 << 0x9A << 0x9B);
		comboBox->addItem(tr("Unary operation"), QList<QVariant>()
						  << 0x95 << 0x96 << 0x7A << 0x7B << 0x97 << 0x98
						  << 0x7C << 0x7D << 0x99);
		comboBox->addItem(tr("Bitwise operation"), QList<QVariant>()
						  << 0x82 << 0x83 << 0x84);
		comboBox->insertSeparator(comboBox->count());
		comboBox->addItem(tr("Seed Random Generator"), QList<QVariant>() << 0x7F);
		comboBox->addItem(tr("Two Byte from two one-byte"), QList<QVariant>() << 0x9C);
		comboBox->addItem(tr("Sinus / Cosinus"), QList<QVariant>() << 0xD4 << 0xD5);
		return;
	case 2:
		comboBox->addItem(tr("Window creation"), QList<QVariant>() << 0x50);
		comboBox->addItem(tr("Resizes/Repositions a window"), QList<QVariant>() << 0x2F);
		comboBox->addItem(tr("Set line count in window"), QList<QVariant>() << 0x55);
		comboBox->addItem(tr("Move a window"), QList<QVariant>() << 0x51);
		comboBox->addItem(tr("Set window type"), QList<QVariant>() << 0x52);
		comboBox->addItem(tr("Reset a window"), QList<QVariant>() << 0x53);
		comboBox->addItem(tr("Close a window (1)"), QList<QVariant>() << 0x2E);
		comboBox->addItem(tr("Close a window (2)"), QList<QVariant>() << 0x54);
		comboBox->insertSeparator(comboBox->count());
		comboBox->addItem(tr("Numerical Display"), QList<QVariant>() << OpcodeKey::WSPCL);
		comboBox->addItem(tr("Set numerical display value"), QList<QVariant>() << OpcodeKey::WNUMB);
		comboBox->addItem(tr("Countdown"), QList<QVariant>() << OpcodeKey::STTIM);
		comboBox->addItem(tr("Set window variable"), QList<QVariant>() << OpcodeKey::MPARA << OpcodeKey::MPRA2);
		comboBox->insertSeparator(comboBox->count());
		comboBox->addItem(tr("Set window Text"), QList<QVariant>() << 0x40);
		comboBox->addItem(tr("Ask Question"), QList<QVariant>() << 0x48);
		comboBox->insertSeparator(comboBox->count());
		comboBox->addItem(tr("Set map name"), QList<QVariant>() << 0x43);
		comboBox->addItem(tr("Enable/Disable menu"), QList<QVariant>() << 0x4A);
		comboBox->addItem(tr("Get window color"), QList<QVariant>() << 0x56);
		comboBox->addItem(tr("Set window color"), QList<QVariant>() << 0x57);
		return;
	case 3:
		comboBox->addItem(tr("HP/MP Maximum (1)"), QList<QVariant>() << 0x3C);
		comboBox->addItem(tr("HP/MP Maximum (2)"), QList<QVariant>() << 0x3D);
		comboBox->addItem(tr("HP/MP Maximum with Status Clear"), QList<QVariant>() << 0x3E);
		comboBox->addItem(tr("HP/MP Maximum (3)"), QList<QVariant>() << 0x3F);
		comboBox->insertSeparator(comboBox->count());
		comboBox->addItem(tr("Increase MP"), QList<QVariant>() << 0x45);
		comboBox->addItem(tr("Decrease MP"), QList<QVariant>() << 0x47);
		comboBox->addItem(tr("Increase HP"), QList<QVariant>() << 0x4D);
		comboBox->addItem(tr("Decrease HP"), QList<QVariant>() << 0x4F);
		comboBox->addItem(tr("Add gil"), QList<QVariant>() << 0x39);
		comboBox->addItem(tr("Remove gil"), QList<QVariant>() << 0x3A);
		comboBox->addItem(tr("Amount of gil"), QList<QVariant>() << 0x3B);
		comboBox->addItem(tr("Add Item"), QList<QVariant>() << 0x58);
		comboBox->addItem(tr("Remove item"), QList<QVariant>() << 0x59);
		comboBox->addItem(tr("Amount of item"), QList<QVariant>() << 0x5A);
		comboBox->addItem(tr("Add Materia"), QList<QVariant>() << 0x5B);
		comboBox->addItem(tr("Remove Materia"), QList<QVariant>() << 0x5C);
		comboBox->addItem(tr("Amount of Materia"), QList<QVariant>() << 0x5D);
		comboBox->insertSeparator(comboBox->count());
		comboBox->addItem(tr("Party Change"), QList<QVariant>() << 0xCA);
		comboBox->addItem(tr("Add Character to the party"), QList<QVariant>() << 0xC8);
		comboBox->addItem(tr("Remove Character from the party"), QList<QVariant>() << 0xC9);
		comboBox->addItem(tr("Save party"), QList<QVariant>() << 0x0A);
		comboBox->addItem(tr("Load party"), QList<QVariant>() << 0x0B);
		comboBox->addItem(tr("Create/Delete character"), QList<QVariant>() << 0xCD);
		comboBox->addItem(tr("Lock character"), QList<QVariant>() << 0xCE);
		comboBox->addItem(tr("Unlock character"), QList<QVariant>() << 0xCF);
		return;
	case 4:
		comboBox->addItem(tr("Set Field Model"), QList<QVariant>() << 0xA1);
		comboBox->addItem(tr("Set character to Field Model"), QList<QVariant>() << 0xA0);
		comboBox->addItem(tr("Group control"), QList<QVariant>() << 0xBF);
		comboBox->insertSeparator(comboBox->count());//Get
		comboBox->addItem(tr("Get Group Direction (Dir)"), QList<QVariant>() << 0xB7);
		comboBox->addItem(tr("Get Party Member Direction (Dir)"), QList<QVariant>() << 0x73);
		comboBox->addItem(tr("Get Group Triangle ID (I)"), QList<QVariant>() << 0xB9);
		comboBox->addItem(tr("Get Party Member Character ID (I)"), QList<QVariant>() << 0x74);
		comboBox->addItem(tr("Get Group coordinates (X,Y)"), QList<QVariant>() << 0xB8);
		comboBox->addItem(tr("Get Group coordinates (X,Y,Z,I)"), QList<QVariant>() << 0xC1);
		comboBox->addItem(tr("Get Party coordinates (X,Y,Z,I)"), QList<QVariant>() << 0x75);
		comboBox->insertSeparator(comboBox->count());//Place
		comboBox->addItem(tr("Place (X,Y,Z)"), QList<QVariant>() << 0xA7);
		comboBox->addItem(tr("Place (X,Y,I)"), QList<QVariant>() << 0xA6);
		comboBox->addItem(tr("Place (X,Y,Z,I)"), QList<QVariant>() << 0xA5);
		comboBox->addItem(tr("Place (Dir)"), QList<QVariant>() << 0xB3);
		comboBox->insertSeparator(comboBox->count());//Move
		comboBox->addItem(tr("Move"), QList<QVariant>() << 0xA8);
		comboBox->addItem(tr("Move without animation"), QList<QVariant>() << 0xAD);
		comboBox->addItem(tr("Move without animation or rotation"), QList<QVariant>() << 0xA9);
		comboBox->addItem(tr("Move to Group"), QList<QVariant>() << 0xAA);
		comboBox->addItem(tr("Move to Party Member"), QList<QVariant>() << 0x2A);
		comboBox->insertSeparator(comboBox->count());//rotation
		comboBox->addItem(tr("Turn"), QList<QVariant>() << 0xB4);
		comboBox->addItem(tr("Direction to Group"), QList<QVariant>() << 0xB6);
		comboBox->addItem(tr("Direction to Party Member"), QList<QVariant>() << 0x34);
		comboBox->addItem(tr("Inversed Turn"), QList<QVariant>() << 0xB5);
		comboBox->addItem(tr("Direction (inversed) to Group"), QList<QVariant>() << 0xAB);
		comboBox->addItem(tr("Direction (inversed) to Party Member"), QList<QVariant>() << 0x35);
		comboBox->addItem(tr("Wait for Turn"), QList<QVariant>() << 0xDE);
		comboBox->insertSeparator(comboBox->count());//Animation
		comboBox->addItem(tr("Play animation (loop)"), QList<QVariant>() << 0xA2);
		comboBox->addItem(tr("Play animation (1)"), QList<QVariant>() << 0xAF);
		comboBox->addItem(tr("Play animation (2)"), QList<QVariant>() << 0xBA);
		comboBox->addItem(tr("Play animation and return (1)"), QList<QVariant>() << 0xA3);
		comboBox->addItem(tr("Play animation and return (2)"), QList<QVariant>() << 0xAE);
		comboBox->addItem(tr("Play partial animation (1)"), QList<QVariant>() << 0xB1);
		comboBox->addItem(tr("Play partial animation (2)"), QList<QVariant>() << 0xBC);
		comboBox->addItem(tr("Play partial animation and return (1)"), QList<QVariant>() << 0xB0);
		comboBox->addItem(tr("Play partial animation and return (2)"), QList<QVariant>() << 0xBB);
		comboBox->addItem(tr("Play Stand/Walk/Run animation"), QList<QVariant>() << 0xDC);
		comboBox->addItem(tr("Play jump animation"), QList<QVariant>() << 0xC0);
		comboBox->addItem(tr("Play climb animation"), QList<QVariant>() << 0xC2);
		comboBox->addItem(tr("Party field join"), QList<QVariant>() << 0x08);
		comboBox->addItem(tr("Party field split"), QList<QVariant>() << 0x09);
		comboBox->addItem(tr("Stop animation"), QList<QVariant>() << 0xDD);
		comboBox->addItem(tr("Wait for animation"), QList<QVariant>() << 0xAC);
		comboBox->insertSeparator(comboBox->count());//Character Graphics KAWAI
		comboBox->addItem(tr("Character Graphics - Eye open/close"), QList<QVariant>() << 0x0028);
		comboBox->addItem(tr("Character Graphics - Activate/Deactivate blending"), QList<QVariant>() << 0x0128);
		comboBox->addItem(tr("Character Graphics - ??? - AMBNT"), QList<QVariant>() << 0x0228);
		comboBox->addItem(tr("Character Graphics - ??? (1)"), QList<QVariant>() << 0x0328);
		comboBox->addItem(tr("Character Graphics - ??? (2)"), QList<QVariant>() << 0x0428);
		comboBox->addItem(tr("Character Graphics - ??? (3)"), QList<QVariant>() << 0x0528);
		comboBox->addItem(tr("Character Graphics - LIGHT"), QList<QVariant>() << 0x0628);
		comboBox->addItem(tr("Character Graphics - ??? (4)"), QList<QVariant>() << 0x0728);
		comboBox->addItem(tr("Character Graphics - ??? (5)"), QList<QVariant>() << 0x0828);
		comboBox->addItem(tr("Character Graphics - ??? (6)"), QList<QVariant>() << 0x0928);
		comboBox->addItem(tr("Character Graphics - SBOBJ"), QList<QVariant>() << 0x0A28);
		comboBox->addItem(tr("Character Graphics - ??? (7)"), QList<QVariant>() << 0x0B28);
		comboBox->addItem(tr("Character Graphics - ??? (8)"), QList<QVariant>() << 0x0C28);
		comboBox->addItem(tr("Character Graphics - SHINE"), QList<QVariant>() << 0x0D28);
		comboBox->addItem(tr("Character Graphics - RESET"), QList<QVariant>() << 0xFF28);
		comboBox->addItem(tr("Wait For Character Graphics"), QList<QVariant>() << 0x29);
		comboBox->insertSeparator(comboBox->count());//offset Object
		comboBox->addItem(tr("OFST"), QList<QVariant>() << 0xC3);
		comboBox->addItem(tr("OFSTW"), QList<QVariant>() << 0xC4);
		comboBox->insertSeparator(comboBox->count());//paramètres
		comboBox->addItem(tr("Movement Speed"), QList<QVariant>() << 0xB2);
		comboBox->addItem(tr("Animation Speed"),QList<QVariant>() <<  0xBD);
		comboBox->addItem(tr("Hide/Display Field Model"), QList<QVariant>() << 0xA4);
		comboBox->addItem(tr("Enable/Disable rotation"), QList<QVariant>() << 0xDB);
		comboBox->addItem(tr("Character Blink"), QList<QVariant>() << 0x26);
		comboBox->addItem(tr("Talk On/Off"), QList<QVariant>() << 0x7E);
		comboBox->addItem(tr("Contact On/Off"), QList<QVariant>() << 0xC7);
		comboBox->addItem(tr("Talk Range (8-bit)"), QList<QVariant>() << 0xC5);
		comboBox->addItem(tr("Contact range (8-bit)"), QList<QVariant>() << 0xC6);
		comboBox->addItem(tr("Talk Range (16-bit)"), QList<QVariant>() << 0xD6);
		comboBox->addItem(tr("Contact range (16-bit)"), QList<QVariant>() << 0xD7);
		return;
	case 5:
		comboBox->addItem(tr("Create line"), QList<QVariant>() << 0xD0);
		comboBox->addItem(tr("Set line"), QList<QVariant>() << 0xD3);
		comboBox->addItem(tr("Line On/Off"), QList<QVariant>() << 0xD1);
		comboBox->addItem(tr("Triangle On/Off"), QList<QVariant>() << 0x6D);
		comboBox->insertSeparator(comboBox->count());//???
		comboBox->addItem(tr("SLIP"), QList<QVariant>() << 0x2B);
		return;
	case 6:
		comboBox->addItem(tr("Move background Z layer"), QList<QVariant>() << 0x2C);
		comboBox->addItem(tr("Animate background layer"), QList<QVariant>() << 0x2D);
		comboBox->addItem(tr("Show a background state"), QList<QVariant>() << 0xE0);
		comboBox->addItem(tr("Hide background state"), QList<QVariant>() << 0xE1);
		comboBox->addItem(tr("Show next background state"), QList<QVariant>() << 0xE2);
		comboBox->addItem(tr("Show previous background state"), QList<QVariant>() << 0xE3);
		comboBox->addItem(tr("Hide background parameters"), QList<QVariant>() << 0xE4);
		comboBox->insertSeparator(comboBox->count());
		comboBox->addItem(tr("Store Palette"), QList<QVariant>() << 0xE5);
		comboBox->addItem(tr("Store Palette (S)"), QList<QVariant>() << 0xEB);
		comboBox->addItem(tr("Load Palette"), QList<QVariant>() << 0xE6);
		comboBox->addItem(tr("Load Palette (S)"), QList<QVariant>() << 0xEC);
		comboBox->addItem(tr("Copy Palette"), QList<QVariant>() << 0xE7);
		comboBox->addItem(tr("Copy Palette (2)"), QList<QVariant>() << 0xED);
		comboBox->addItem(tr("Partial Copy Palette"), QList<QVariant>() << 0xE8);
		comboBox->addItem(tr("Partial Copy Palette (2)"), QList<QVariant>() << 0xEE);
		comboBox->addItem(tr("Multiply Palette"), QList<QVariant>() << 0xDF);
		comboBox->addItem(tr("Multiply Palette (2)"), QList<QVariant>() << 0xEA);
		comboBox->addItem(tr("Add Palette"), QList<QVariant>() << 0xE9);
		comboBox->addItem(tr("Add Palette (2)"), QList<QVariant>() << 0xEF);
		return;
	case 7:
		comboBox->addItem(tr("Scroll to playable character"), QList<QVariant>() << 0x65);
		comboBox->addItem(tr("Scroll to party member"), QList<QVariant>() << 0x6F);
		comboBox->addItem(tr("Scroll to group"), QList<QVariant>() << 0x63);
		comboBox->insertSeparator(comboBox->count());
		comboBox->addItem(tr("Scroll (X,Y)"), QList<QVariant>() << 0x64);
		comboBox->addItem(tr("Scroll (X,Y,Smooth)"), QList<QVariant>() << 0x66);
		comboBox->addItem(tr("Scroll (X,Y,Linear)"), QList<QVariant>() << 0x68);
		comboBox->addItem(tr("SCRLO"), QList<QVariant>() << 0x61);
		comboBox->addItem(tr("SCRLC"), QList<QVariant>() << 0x62);
		comboBox->insertSeparator(comboBox->count());
		comboBox->addItem(tr("Shake"), QList<QVariant>() << 0x5E);
		comboBox->addItem(tr("Wait for scroll"), QList<QVariant>() << 0x67);
		comboBox->insertSeparator(comboBox->count());
		comboBox->addItem(tr("VWOFT"), QList<QVariant>() << 0x6A);
		comboBox->addItem(tr("FADE"), QList<QVariant>() << 0x6B);
		comboBox->addItem(tr("FADEW"), QList<QVariant>() << 0x6C);
		comboBox->addItem(tr("NFADE"), QList<QVariant>() << 0x25);
		return;
	case 8:
		comboBox->addItem(tr("Play sound"), QList<QVariant>() << 0xF1);
		comboBox->addItem(tr("Alter sound (8-bit)"), QList<QVariant>() << 0xF2);
		comboBox->addItem(tr("Alter sound (16-bit)"), QList<QVariant>() << 0xDA);
		comboBox->insertSeparator(comboBox->count());
		comboBox->addItem(tr("Play music"), QList<QVariant>() << 0xF0);
		comboBox->addItem(tr("Play temporary music"), QList<QVariant>() << 0xF3);
		comboBox->addItem(tr("MUSVM (unused)"), QList<QVariant>() << 0xF4);
		comboBox->addItem(tr("Music Unlock/Lock"), QList<QVariant>() << 0xF5);
		comboBox->addItem(tr("Battle music"), QList<QVariant>() << 0xF6);
		comboBox->addItem(tr("CHMPH (unused)"), QList<QVariant>() << 0xF7);
		comboBox->addItem(tr("Get music status"), QList<QVariant>() << 0xFE);
		comboBox->addItem(tr("Field music"), QList<QVariant>() << 0xFC);
		comboBox->addItem(tr("Play basic akao operation"), QList<QVariant>() << 0xFD);
		comboBox->insertSeparator(comboBox->count());
		comboBox->addItem(tr("Set next movie"), QList<QVariant>() << 0xF8);
		comboBox->addItem(tr("Play movie"), QList<QVariant>() << 0xF9);
		comboBox->addItem(tr("MVIEF"), QList<QVariant>() << 0xFA);
		comboBox->addItem(tr("MVCAM"), QList<QVariant>() << 0xFB);
		comboBox->addItem(tr("BGMOVIE"), QList<QVariant>() << 0x27);
		return;
	case 9:
		comboBox->addItem(tr("Play tutorial"), QList<QVariant>() << 0x21);
		comboBox->addItem(tr("Display a menu"), QList<QVariant>() << 0x49);
		comboBox->addItem(tr("Disc change screen"), QList<QVariant>() << 0x0E);
		comboBox->addItem(tr("Minigame"), QList<QVariant>() << 0x20);
		comboBox->addItem(tr("Battle result load"), QList<QVariant>() << 0x23);
		comboBox->addItem(tr("Battle Table"), QList<QVariant>() << 0x4B);
		comboBox->addItem(tr("Change Map"), QList<QVariant>() << 0x60);
		comboBox->addItem(tr("Last Map ID"), QList<QVariant>() << 0x6E);
		comboBox->addItem(tr("Start Battle"), QList<QVariant>() << 0x70);
		comboBox->addItem(tr("Battle On/Off"), QList<QVariant>() << 0x71);
		comboBox->addItem(tr("Battle mode"), QList<QVariant>() << 0x72);
		comboBox->addItem(tr("Battle mode (2)"), QList<QVariant>() << 0x22);
		comboBox->addItem(tr("Map Jump On/Off"), QList<QVariant>() << 0xD2);
		comboBox->addItem(tr("Character movability On/Off"), QList<QVariant>() << 0x33);
		comboBox->addItem(tr("Preload field Map"), QList<QVariant>() << 0xD8);
		comboBox->addItem(tr("PMJMP2"), QList<QVariant>() << 0xD9);
		comboBox->addItem(tr("Game Over"), QList<QVariant>() << 0xFF);
		comboBox->addItem(tr("[DLPB's custom opcode] Write/Read to entire Savemap"), QList<QVariant>() << 0x1A);
		comboBox->addItem(tr("[DLPB's custom opcode] Write to any memory address via array"), QList<QVariant>() << 0x1C);
		comboBox->addItem(tr("SPECIAL - Cursor On/Off"), QList<QVariant>() << 0xF50F);
		comboBox->addItem(tr("SPECIAL - PNAME"), QList<QVariant>() << 0xF60F);
		comboBox->addItem(tr("SPECIAL - Game Speed"), QList<QVariant>() << 0xF70F);
		comboBox->addItem(tr("SPECIAL - Messages Speed"), QList<QVariant>() << 0xF80F);
		comboBox->addItem(tr("SPECIAL - Full Materia"), QList<QVariant>() << 0xF90F);
		comboBox->addItem(tr("SPECIAL - Full Item"), QList<QVariant>() << 0xFA0F);
		comboBox->addItem(tr("SPECIAL - Battle On/Off"), QList<QVariant>() << 0xFB0F);
		comboBox->addItem(tr("SPECIAL - Character movability On/Off"), QList<QVariant>() << 0xFC0F);
		comboBox->addItem(tr("SPECIAL - Rename character"), QList<QVariant>() << 0xFD0F);
		comboBox->addItem(tr("SPECIAL - Clear Game"), QList<QVariant>() << 0xFE0F);
		comboBox->addItem(tr("SPECIAL - Clear Items"), QList<QVariant>() << 0xFF0F);
		return;
	case 10:
		comboBox->addItem(tr("MPDSP"), QList<QVariant>() << 0x69);
		comboBox->addItem(tr("SETX"), QList<QVariant>() << 0x9D);
		comboBox->addItem(tr("GETX"), QList<QVariant>() << 0x9E);
		comboBox->addItem(tr("SEARCHX"), QList<QVariant>() << 0x9F);
		return;
	}
}

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
#include "TextManager.h"
#include "TextHighlighter.h"
#include "core/Config.h"
#include "Data.h"

#include <FF7Text>
#include <FF7String>
#include <ListWidget>
#include <FF7Char>

QIcon TextManager::winIcon;
QIcon TextManager::noWinIcon;

TextManager::TextManager(QWidget *parent) :
    QDialog(parent, Qt::Tool), scriptsAndTexts(nullptr)
{
	setWindowTitle(tr("Texts"));

	dispUnusedText = new QCheckBox(tr("Show unused texts"), this);
	dispUnusedText->setChecked(Config::value("dispUnusedText", true).toBool());

	ListWidget *listWidget = new ListWidget(this);
	listWidget->addAction(ListWidget::Add, tr("Add text"));
	listWidget->addAction(ListWidget::Remove, tr("Remove text"));
	liste1 = listWidget->listWidget();

	QAction *action;
	toolBar = new QToolBar(this);
	toolBar->setIconSize(QSize(16, 16));
	action = toolBar->addAction(FF7Char::icon(0), Data::char_names.at(0));
	action->setData("{CLOUD}");
	action = toolBar->addAction(FF7Char::icon(1), Data::char_names.at(1));
	action->setData("{BARRET}");
	action = toolBar->addAction(FF7Char::icon(2), Data::char_names.at(2));
	action->setData("{TIFA}");
	action = toolBar->addAction(FF7Char::icon(3), Data::char_names.at(3));
	action->setData("{AERITH}");
	action = toolBar->addAction(FF7Char::icon(4), Data::char_names.at(4));
	action->setData("{RED XIII}");
	action = toolBar->addAction(FF7Char::icon(5), Data::char_names.at(5));
	action->setData("{YUFFIE}");
	action = toolBar->addAction(FF7Char::icon(6), Data::char_names.at(6));
	action->setData("{CAIT SITH}");
	action = toolBar->addAction(FF7Char::icon(7), Data::char_names.at(7));
	action->setData("{VINCENT}");
	action = toolBar->addAction(FF7Char::icon(8), Data::char_names.at(8));
	action->setData("{CID}");
	toolBar->addSeparator();
	action = toolBar->addAction(QIcon::fromTheme(QStringLiteral("member-1")), tr("Member 1"));
	action->setData("{MEMBER 1}");
	action = toolBar->addAction(QIcon::fromTheme(QStringLiteral("member-2")), tr("Member 2"));
	action->setData("{MEMBER 2}");
	action = toolBar->addAction(QIcon::fromTheme(QStringLiteral("member-3")), tr("Member 3"));
	action->setData("{MEMBER 3}");
	toolBar->addSeparator();
	action = toolBar->addAction(QIcon::fromTheme(QStringLiteral("kmouth-phrase-new")), tr("New Page"));
	action->setData("\n{NEW PAGE}\n");
	action = toolBar->addAction(QIcon::fromTheme(QStringLiteral("question")), tr("Choice"));
	action->setData("{CHOICE}");

	toolBar2 = new QToolBar(this);
	toolBar2->setIconSize(QSize(16, 16));
	action = toolBar2->addAction(QIcon::fromTheme(QStringLiteral("greyblock")), tr("Grey"));
	action->setData("{GREY}");
	action = toolBar2->addAction(QIcon::fromTheme(QStringLiteral("blueblock")), tr("Blue"));
	action->setData("{BLUE}");
	action = toolBar2->addAction(QIcon::fromTheme(QStringLiteral("redblock")), tr("Red"));
	action->setData("{RED}");
	action = toolBar2->addAction(QIcon::fromTheme(QStringLiteral("purpleblock")), tr("Purple"));
	action->setData("{PURPLE}");
	action = toolBar2->addAction(QIcon::fromTheme(QStringLiteral("greenblock")), tr("Green"));
	action->setData("{GREEN}");
	action = toolBar2->addAction(QIcon::fromTheme(QStringLiteral("cyanblock")), tr("Cyan"));
	action->setData("{CYAN}");
	action = toolBar2->addAction(QIcon::fromTheme(QStringLiteral("yellowblock")), tr("Yellow"));
	action->setData("{YELLOW}");
	action = toolBar2->addAction(QIcon::fromTheme(QStringLiteral("whiteblock")), tr("White"));
	action->setData("{WHITE}");
	action = toolBar2->addAction(QIcon::fromTheme(QStringLiteral("blinkblock")), tr("Blink"));
	action->setData("{BLINK}");
	action = toolBar2->addAction(QIcon::fromTheme(QStringLiteral("multicolorblock")), tr("Multicolor"));
	action->setData("{MULTICOLOUR}");
	toolBar2->addSeparator();

	menu1 = new QMenu(this);
	action = menu1->addAction(tr("Pause"));
	action->setData("{PAUSE}");
	action = menu1->addAction(tr("Pause 0"));
	action->setData("{PAUSE000}");
	action = menu1->addAction(tr("Pause 5"));
	action->setData("{PAUSE005}");
	action = menu1->addAction(tr("Pause 10"));
	action->setData("{PAUSE010}");
	action = menu1->addAction(tr("Pause 15"));
	action->setData("{PAUSE015}");
	action = menu1->addAction(tr("Pause 20"));
	action->setData("{PAUSE020}");
	action = menu1->addAction(tr("Pause 30"));
	action->setData("{PAUSE030}");
	action = menu1->addAction(tr("Pause 40"));
	action->setData("{PAUSE040}");
	action = menu1->addAction(tr("Pause 50"));
	action->setData("{PAUSE050}");
	action = menu1->addAction(tr("Pause 60"));
	action->setData("{PAUSE060}");
	action = toolBar2->addAction(QIcon::fromTheme(QStringLiteral("media-playback-pause")), tr("Pauses"));
	action->setData("{PAUSE}");
	action->setMenu(menu1);
	toolBar2->addSeparator();

	menuVars = new QMenu(this);
	action = menuVars->addAction(tr("Var10"));
	action->setData("{VARDEC}");
	action = menuVars->addAction(tr("Var16"));
	action->setData("{VARHEX}");
	action = menuVars->addAction(tr("Var10r"));
	action->setData("{VARDECR}");
	action = toolBar2->addAction(tr("Vars"));
	action->setData("{VARDEC}");
	action->setMenu(menuVars);
	toolBar2->addSeparator();

	menuKeys = new QMenu(this);
	action = menuKeys->addAction(QIcon(QStringLiteral(":/psxButtons/%1/circle").arg(Config::iconThemeColor())), tr("Circle"));
	action->setData("{CIRCLE}");
	action = menuKeys->addAction(QIcon(QStringLiteral(":/psxButtons/%1/triangle").arg(Config::iconThemeColor())), tr("Triangle"));
	action->setData("{TRIANGLE}");
	action = menuKeys->addAction(QIcon(QStringLiteral(":/psxButtons/%1/square").arg(Config::iconThemeColor())), tr("Square"));
	action->setData("{SQUARE}");
	action = menuKeys->addAction(QIcon(QStringLiteral(":/psxButtons/%1/cross").arg(Config::iconThemeColor())), tr("Cross"));
	action->setData("{CROSS}");
	action = toolBar2->addAction(tr("Keys"));
	action->setMenu(menuKeys);
	toolBar2->addSeparator();

	menu2 = new QMenu(this);
	action = menu2->addAction(tr("Scrolling"));
	action->setData("{SCROLLING}");
	action = menu2->addAction(tr("Spaced characters"));
	action->setData("{SPACED CHARACTERS}");
	action = menu2->addAction(tr("Memory access"));
	action->setData("{MEMORY:var[1][0];size=1}");
	action = menu2->addAction(tr("New Page²"));
	action->setData("\n{NEW PAGE 2}\n");
	action = toolBar2->addAction(tr("Others"));
	action->setMenu(menu2);

	textEdit = new QPlainTextEdit(this);
	textEdit->setTabStopDistance(40);
	textEdit->setLineWrapMode(QPlainTextEdit::NoWrap);
	new TextHighlighter(textEdit->document());

	QVBoxLayout *toolBars = new QVBoxLayout();
	toolBars->addWidget(toolBar);
	toolBars->addWidget(toolBar2);
	toolBars->setContentsMargins(QMargins());
	toolBars->setSpacing(0);

	QWidget *groupTextPreview = new QWidget(this);
	textPreview = new TextPreview(groupTextPreview);

	prevPage = new QToolButton(groupTextPreview);
	prevPage->setArrowType(Qt::UpArrow);
	prevPage->setToolButtonStyle(Qt::ToolButtonIconOnly);
	nextPage = new QToolButton(groupTextPreview);
	nextPage->setArrowType(Qt::DownArrow);
	nextPage->setToolButtonStyle(Qt::ToolButtonIconOnly);
	textPage = new QLabel(groupTextPreview);
	textPage->setTextFormat(Qt::PlainText);

	prevWin = new QToolButton(groupTextPreview);
	prevWin->setArrowType(Qt::UpArrow);
	prevWin->setToolButtonStyle(Qt::ToolButtonIconOnly);
	nextWin = new QToolButton(groupTextPreview);
	nextWin->setArrowType(Qt::DownArrow);
	nextWin->setToolButtonStyle(Qt::ToolButtonIconOnly);
	textWin = new QLabel(groupTextPreview);
	textWin->setTextFormat(Qt::PlainText);

	xCoord = new QSpinBox(groupTextPreview);
	yCoord = new QSpinBox(groupTextPreview);
	xCoord->setRange(-32768, 32767);
	yCoord->setRange(-32768, 32767);

	wSize = new QSpinBox(groupTextPreview);
	hSize = new QSpinBox(groupTextPreview);
	wSize->setRange(0, 65535);
	hSize->setRange(0, 65535);

	hAlign = new QPushButton(tr("Align horizontally"), groupTextPreview);
	vAlign = new QPushButton(tr("Align vertically"), groupTextPreview);
	autoSize = new QPushButton(tr("Autosize"), groupTextPreview);

	QGridLayout *buttonLayout = new QGridLayout;
	buttonLayout->addWidget(hAlign, 0, 1);
	buttonLayout->addWidget(vAlign, 1, 1);
	buttonLayout->addWidget(autoSize, 1, 0);
	buttonLayout->setContentsMargins(QMargins());

	xLabel = new QLabel(tr("X"));
	yLabel = new QLabel(tr("Y"));
	wLabel = new QLabel(tr("W"));
	hLabel = new QLabel(tr("H"));

	QGridLayout *layoutTextRect = new QGridLayout;
	layoutTextRect->addWidget(xLabel, 0, 0);
	layoutTextRect->addWidget(xCoord, 0, 1);
	layoutTextRect->addWidget(yLabel, 0, 2);
	layoutTextRect->addWidget(yCoord, 0, 3);
	layoutTextRect->addWidget(wLabel, 1, 0);
	layoutTextRect->addWidget(wSize, 1, 1);
	layoutTextRect->addWidget(hLabel, 1, 2);
	layoutTextRect->addWidget(hSize, 1, 3);
	layoutTextRect->setColumnStretch(1, 1);
	layoutTextRect->setColumnStretch(3, 1);
	layoutTextRect->setContentsMargins(QMargins());

	QGridLayout *layoutTextPreview = new QGridLayout(groupTextPreview);
	layoutTextPreview->addWidget(textPreview, 0, 0, 5, 1, Qt::AlignLeft | Qt::AlignTop);
	layoutTextPreview->addWidget(prevPage, 0, 1, 1, 2, Qt::AlignLeft | Qt::AlignTop);
	layoutTextPreview->addWidget(textPage, 1, 1, 1, 2, Qt::AlignLeft | Qt::AlignTop);
	layoutTextPreview->addWidget(nextPage, 2, 1, 1, 2, Qt::AlignLeft | Qt::AlignTop);
	layoutTextPreview->addWidget(prevWin, 0, 3, 1, 2, Qt::AlignLeft | Qt::AlignTop);
	layoutTextPreview->addWidget(textWin, 1, 3, 1, 2, Qt::AlignLeft | Qt::AlignTop);
	layoutTextPreview->addWidget(nextWin, 2, 3, 1, 2, Qt::AlignLeft | Qt::AlignTop);
	layoutTextPreview->addLayout(layoutTextRect, 3, 1, 1, 4);
	layoutTextPreview->addLayout(buttonLayout, 4, 1, 1, 4, Qt::AlignRight | Qt::AlignBottom);
	layoutTextPreview->setRowStretch(2, 1);
	layoutTextPreview->setContentsMargins(QMargins());

	groupTextPreview->setFixedHeight(224);

	QGridLayout *layout = new QGridLayout();
	layout->addWidget(dispUnusedText, 0, 0, 1, 2);
	layout->addWidget(listWidget, 1, 0, 3, 1);
	layout->addLayout(toolBars, 1, 1);
	layout->addWidget(textEdit, 2, 1);
	layout->addWidget(groupTextPreview, 3, 1);
	liste1->setFocus();
	setLayout(layout);
	adjustSize();

	connect(listWidget, &ListWidget::addTriggered, this, &TextManager::addText);
	connect(listWidget, &ListWidget::removeTriggered, this, &TextManager::delText);
	connect(liste1, &QListWidget::currentItemChanged, this, &TextManager::selectText);
	connect(dispUnusedText, &QCheckBox::toggled, this, &TextManager::showList);
	connect(toolBar, &QToolBar::actionTriggered, this, &TextManager::insertTag);
	connect(toolBar2, &QToolBar::actionTriggered, this, &TextManager::insertTag);
	connect(menu1, &QMenu::triggered, this, &TextManager::insertTag);
	connect(menu2, &QMenu::triggered, this, &TextManager::insertTag);
	connect(menuVars, &QMenu::triggered, this, &TextManager::insertTag);
	connect(menuKeys, &QMenu::triggered, this, &TextManager::insertTag);
	connect(textEdit, &QPlainTextEdit::textChanged, this, &TextManager::setTextChanged);
	connect(prevPage, &QPushButton::clicked, this, &TextManager::prevTextPreviewPage);
	connect(nextPage, &QPushButton::clicked, this, &TextManager::nextTextPreviewPage);
	connect(prevWin, &QPushButton::clicked, this, &TextManager::prevTextPreviewWin);
	connect(nextWin, &QPushButton::clicked, this, &TextManager::nextTextPreviewWin);
	connect(textPreview, &TextPreview::positionChanged, this, &TextManager::changePosition);
	connect(textPreview, &TextPreview::pageChanged, this, &TextManager::changeTextPreviewPage);
	connect(xCoord, &QSpinBox::valueChanged, this, &TextManager::changeXCoord);
	connect(yCoord, &QSpinBox::valueChanged, this, &TextManager::changeYCoord);
	connect(wSize, &QSpinBox::valueChanged, this, &TextManager::changeWSize);
	connect(hSize, &QSpinBox::valueChanged, this, &TextManager::changeHSize);
	connect(hAlign, &QPushButton::clicked, this, &TextManager::alignHorizontally);
	connect(vAlign, &QPushButton::clicked, this, &TextManager::alignVertically);
	connect(autoSize, &QPushButton::clicked, this, &TextManager::resizeWindow);
}

void TextManager::focusInEvent(QFocusEvent *)
{
	liste1->setFocus();
}

void TextManager::setField(Field *field, bool reload)
{
	if (!field
	        || (!reload && this->scriptsAndTexts == field->scriptsAndTexts())
	        || !field->scriptsAndTexts()->isOpen()) {
		return;
	}

	clear();
	this->scriptsAndTexts = field->scriptsAndTexts();
	usedTexts = scriptsAndTexts->listUsedTexts();
	showList();
	liste1->setCurrentRow(0);
	selectText(liste1->item(0));
}

void TextManager::clear()
{
	scriptsAndTexts = nullptr;
	usedTexts.clear();
	liste1->clear();
	textEdit->clear();
	textPreview->clear();
}

void TextManager::setTextChanged()
{
	if (!scriptsAndTexts) {
		return;
	}

	QListWidgetItem *item = liste1->currentItem();

	if (item == nullptr) {
		return;
	}

	QString newText = textEdit->toPlainText();
	int textId = item->data(Qt::UserRole).toInt();
	const FF7String &t = scriptsAndTexts->text(textId);
	bool jp = Config::value("jp_txt", false).toBool();

	if (newText != t.text()) {
		scriptsAndTexts->setText(textId, FF7String(newText, jp));
		textPreview->setText(t.data());
		changeTextPreviewPage();
		changeTextPreviewWin();
		emit modified();
	}
}

QList<FF7Window> TextManager::getWindows(int textID) const
{
	QList<FF7Window> windows;
	scriptsAndTexts->listWindows(textID, windows);
	return windows;
}

void TextManager::selectText(QListWidgetItem *item, QListWidgetItem *)
{
	if (!item || !scriptsAndTexts) {
		return;
	}

	int textID = item->data(Qt::UserRole).toInt();
	const FF7String &t = scriptsAndTexts->text(textID);
	textPreview->resetCurrentWin();
	textPreview->setWins(getWindows(textID));
	textPreview->setText(t.data());
	textEdit->setPlainText(FF7Text::toPC(t.data()));
	changeTextPreviewPage();
	changeTextPreviewWin();
}

void TextManager::showList()
{
	if (!scriptsAndTexts) {
		return;
	}
	bool show = dispUnusedText->isChecked();
	liste1->blockSignals(true);
	qsizetype nbTexts = scriptsAndTexts->textCount();
	liste1->clear();

	Config::setValue("dispUnusedText", show);

	if (winIcon.isNull()) {
		QPixmap pix(16, 8);
		QPainter p(&pix);
		TextPreview::drawWindow(&p, pix.width(), pix.height(), TextPreview::WithoutFrame);
		p.end();
		winIcon = QIcon(pix);
		noWinIcon = winIcon.pixmap(pix.width(), pix.height(), QIcon::Disabled);
	}

	for (int i = 0; i < nbTexts; ++i) {
		if (!show && !usedTexts.contains(quint8(i))) {
			continue;
		}

		bool hasWin = !getWindows(i).isEmpty();
		QListWidgetItem *item = new QListWidgetItem(hasWin ? winIcon : noWinIcon, tr("Text %1").arg(i));
		item->setData(Qt::UserRole, i);
		liste1->addItem(item);

		if (!usedTexts.contains(quint8(i))) {
			item->setForeground(Data::color(Data::ColorDisabledForeground));
		}
	}

	liste1->setCurrentRow(0);
	liste1->blockSignals(false);
}

void TextManager::updateText()
{
	selectText(liste1->currentItem());
	if (textPreview->winCount() == 0) {
		textPreview->calcSize();
	} else {
		textPreview->update();
	}
}

void TextManager::updateFromScripts()
{
	usedTexts = scriptsAndTexts->listUsedTexts();

	for (int row = 0; row < liste1->count(); ++row) {
		QListWidgetItem *item = liste1->item(row);
		int textID = item->data(Qt::UserRole).toInt();
		if (!usedTexts.contains(quint8(textID))) {
			item->setForeground(Data::color(Data::ColorDisabledForeground));
		} else {
			// Default foreground
			item->setForeground(palette().brush(QPalette::WindowText));
		}
	}
}

void TextManager::updateNames()
{
	textPreview->updateNames();
}

void TextManager::gotoText(int textID, qsizetype from, qsizetype size)
{
	for (int i=0; i<liste1->count(); ++i) {
		if (textID == liste1->item(i)->data(Qt::UserRole).toInt()) {
			blockSignals(true);
			textEdit->blockSignals(true);
			liste1->setCurrentItem(liste1->item(i));
			selectText(liste1->item(i));
			liste1->scrollToItem(liste1->item(i));
			QTextCursor t = textEdit->textCursor();
			t.setPosition(int(from));
			t.setPosition(int(from + size), QTextCursor::KeepAnchor);
			textEdit->setTextCursor(t);
			textEdit->setFocus();
			blockSignals(false);
			textEdit->blockSignals(false);
		}
	}
}

QString TextManager::selectedText() const
{
	return textEdit->textCursor().selectedText();
}

int TextManager::currentTextId() const
{
	QListWidgetItem *currentItem = liste1->currentItem();
	if (currentItem) {
		return currentItem->data(Qt::UserRole).toInt();
	}
	return -1;
}

int TextManager::currentTextPosition() const
{
	return textEdit->textCursor().position();
}

int TextManager::currentAnchorPosition() const
{
	return textEdit->textCursor().anchor();
}

void TextManager::addText()
{
	if (!scriptsAndTexts || scriptsAndTexts->textCount() >= scriptsAndTexts->maxTextCount()) {
		return;
	}
	QListWidgetItem *item = liste1->currentItem();
	int row = !item ? int(scriptsAndTexts->textCount()) : item->data(Qt::UserRole).toInt() + 1;
	liste1->blockSignals(true);
	scriptsAndTexts->insertText(row, FF7String());
	usedTexts = scriptsAndTexts->listUsedTexts();
	dispUnusedText->setChecked(true);
	showList();
	liste1->setCurrentRow(row < liste1->count() ? row : liste1->count()-1);
	emit modified();
	liste1->blockSignals(false);
}

void TextManager::delText()
{
	if (!scriptsAndTexts) {
		return;
	}
	QListWidgetItem *item = liste1->currentItem();
	if (!item) {
		return;
	}
	int row = item->data(Qt::UserRole).toInt();
	if (usedTexts.contains(quint8(row))) {
		QMessageBox::StandardButton rep = QMessageBox::warning(this, tr("Text used in scripts"), tr("This text is used by one or more scripts on this field.\nRemoving this text may break scripts that reference it.\nAre you sure you want to continue?"), QMessageBox::Yes | QMessageBox::Cancel, QMessageBox::Cancel);
		if (rep == QMessageBox::Cancel) {
			return;
		}
	}
	liste1->blockSignals(true);
	scriptsAndTexts->deleteText(row);
	usedTexts = scriptsAndTexts->listUsedTexts();
	dispUnusedText->setChecked(true);
	showList();
	liste1->setCurrentRow(row < liste1->count() ? row : liste1->count()-1);
	emit modified();
	liste1->blockSignals(false);
}

void TextManager::prevTextPreviewPage()
{
	textPreview->prevPage();
	changeTextPreviewPage();
}

void TextManager::nextTextPreviewPage()
{
	textPreview->nextPage();
	changeTextPreviewPage();
}

void TextManager::changeTextPreviewPage()
{
	int currentPage = textPreview->currentPage();
	qsizetype nbPage = textPreview->pageCount();

	textPage->setText(tr("Page %1/%2").arg(currentPage).arg(nbPage));
	prevPage->setEnabled(currentPage > 1);
	nextPage->setEnabled(currentPage < nbPage);
	textPage->setEnabled(nbPage > 0);
}

void TextManager::prevTextPreviewWin()
{
	textPreview->prevWin();
	changeTextPreviewWin();
}

void TextManager::nextTextPreviewWin()
{
	textPreview->nextWin();
	changeTextPreviewWin();
}

void TextManager::changeTextPreviewWin()
{
	int currentWin = textPreview->currentWin();
	qsizetype nbWin = textPreview->winCount();
	bool currentWinIsValid = nbWin > 0 && textPreview->getWindow().type != NOWIN && textPreview->getWindow().type != OpcodeKey::MPNAM;

	textWin->setText(tr("Window %1/%2").arg(currentWin).arg(nbWin));
	prevWin->setEnabled(currentWin > 1);
	nextWin->setEnabled(currentWin < nbWin);
	textWin->setEnabled(nbWin > 0);
	xCoord->setEnabled(currentWinIsValid);
	xLabel->setEnabled(currentWinIsValid);
	yCoord->setEnabled(currentWinIsValid);
	yLabel->setEnabled(currentWinIsValid);
	wSize->setEnabled(currentWinIsValid);
	wLabel->setEnabled(currentWinIsValid);
	hSize->setEnabled(currentWinIsValid);
	hLabel->setEnabled(currentWinIsValid);
	hAlign->setEnabled(currentWinIsValid);
	vAlign->setEnabled(currentWinIsValid);
	autoSize->setEnabled(currentWinIsValid);

	updateWindowCoord();
}

void TextManager::changePosition(const QPoint &point)
{
	if (!scriptsAndTexts || textPreview->winCount() <= 0
	    || liste1->currentItem() == nullptr) {
		return;
	}

	xCoord->blockSignals(true);
	xCoord->setValue(point.x());
	xCoord->blockSignals(false);
	yCoord->blockSignals(true);
	yCoord->setValue(point.y());
	yCoord->blockSignals(false);

	FF7Window ff7Window = textPreview->getWindow();
	ff7Window.x = qint16(point.x());
	ff7Window.y = qint16(point.y());
	scriptsAndTexts->setWindow(ff7Window);
	textPreview->setWindow(ff7Window);
	emit modified();
	emit opcodeModified(ff7Window.groupID, ff7Window.scriptID, ff7Window.opcodeID);
}

void TextManager::changeSize(const QSize &size)
{
	if (!scriptsAndTexts || textPreview->winCount() <= 0
	    || liste1->currentItem() == nullptr) {
		return;
	}

	wSize->blockSignals(true);
	wSize->setValue(size.width());
	wSize->blockSignals(false);
	hSize->blockSignals(true);
	hSize->setValue(size.height());
	hSize->blockSignals(false);

	FF7Window ff7Window = textPreview->getWindow();
	ff7Window.w = quint16(size.width());
	ff7Window.h = quint16(size.height());
	scriptsAndTexts->setWindow(ff7Window);
	textPreview->setWindow(ff7Window);
	emit modified();
	emit opcodeModified(ff7Window.groupID, ff7Window.scriptID, ff7Window.opcodeID);
}

void TextManager::changeXCoord(int x)
{
	if (!scriptsAndTexts || textPreview->winCount() <= 0
	    || liste1->currentItem() == nullptr) {
		return;
	}

	FF7Window ff7Window = textPreview->getWindow();

	if (ff7Window.x != x) {
		ff7Window.x = qint16(x);

		scriptsAndTexts->setWindow(ff7Window);
		textPreview->setWindow(ff7Window);
		textPreview->update();
		emit modified();
		emit opcodeModified(ff7Window.groupID, ff7Window.scriptID, ff7Window.opcodeID);
	}
}

void TextManager::changeYCoord(int y)
{
	if (!scriptsAndTexts || textPreview->winCount() <= 0
	    || liste1->currentItem() == nullptr) {
		return;
	}

	FF7Window ff7Window = textPreview->getWindow();

	if (ff7Window.y != y) {
		ff7Window.y = qint16(y);

		scriptsAndTexts->setWindow(ff7Window);
		textPreview->setWindow(ff7Window);
		textPreview->update();
		emit modified();
		emit opcodeModified(ff7Window.groupID, ff7Window.scriptID, ff7Window.opcodeID);
	}
}

void TextManager::changeWSize(int w)
{
	if (!scriptsAndTexts || textPreview->winCount() <= 0
	    || liste1->currentItem() == nullptr) {
		return;
	}

	FF7Window ff7Window = textPreview->getWindow();

	if (ff7Window.w != w) {
		ff7Window.w = quint16(w);

		scriptsAndTexts->setWindow(ff7Window);
		textPreview->setWindow(ff7Window);
		textPreview->update();
		emit modified();
		emit opcodeModified(ff7Window.groupID, ff7Window.scriptID, ff7Window.opcodeID);
	}
}

void TextManager::changeHSize(int h)
{
	if (!scriptsAndTexts || textPreview->winCount() <= 0
	    || liste1->currentItem() == nullptr) {
		return;
	}

	FF7Window ff7Window = textPreview->getWindow();

	if (ff7Window.h != h) {
		ff7Window.h = quint16(h);

		scriptsAndTexts->setWindow(ff7Window);
		textPreview->setWindow(ff7Window);
		textPreview->update();
		emit modified();
		emit opcodeModified(ff7Window.groupID, ff7Window.scriptID, ff7Window.opcodeID);
	}
}

void TextManager::updateWindowCoord()
{
	FF7Window ff7Win = textPreview->getWindow();
	xCoord->setValue(ff7Win.x);
	yCoord->setValue(ff7Win.y);
	wSize->setValue(ff7Win.w);
	hSize->setValue(ff7Win.h);
}

void TextManager::insertTag(QAction *action)
{
	if (sender() != action->parentWidget()) {
		return; // toolBar/Menu signals hack
	}
	textEdit->insertPlainText(action->data().toString());
	textEdit->setFocus();
}

void TextManager::align(Qt::Alignment alignment)
{
	if (xCoord->isEnabled()) {
		QPoint point(xCoord->value(), yCoord->value());
		if (alignment.testFlag(Qt::AlignHCenter)) {
			point.setX((textPreview->width() - wSize->value()) / 2);
		}
		if (alignment.testFlag(Qt::AlignVCenter)) {
			point.setY((textPreview->height() - hSize->value()) / 2);
		}
		changePosition(point);
		textPreview->update();
	}
}

void TextManager::resizeWindow()
{
	textPreview->calcSize();
	changeSize(textPreview->getCalculatedSize());
	textPreview->update();
}

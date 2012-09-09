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
#include "TextManager.h"
#include "FF7Text.h"
#include "TextHighlighter.h"
#include "Config.h"
#include "Data.h"

TextManager::TextManager(QWidget *parent) :
	QDialog(parent, Qt::Tool)
{
	setWindowTitle(tr("Textes"));
	QFont font;
	font.setPointSize(8);

	dispUnusedText = new QCheckBox(tr("Afficher les textes non utilisés"), this);
	dispUnusedText->setChecked(Config::value("dispUnusedText", true).toBool());

	liste1 = new QListWidget(this);
	liste1->setFont(font);
	liste1->setFixedWidth(100);

	QAction *action;
	toolBar = new QToolBar(this);
	toolBar->setIconSize(QSize(16, 16));
	action = toolBar->addAction(tr("Nouvelle page"));
	action->setData("\n{NEW PAGE}\n");
	action = toolBar->addAction(QIcon(":/images/icon-cloud.png"), Data::char_names.at(0));
	action->setData("{CLOUD}");
	action = toolBar->addAction(QIcon(":/images/icon-barret.png"), Data::char_names.at(1));
	action->setData("{BARRET}");
	action = toolBar->addAction(QIcon(":/images/icon-tifa.png"), Data::char_names.at(2));
	action->setData("{TIFA}");
	action = toolBar->addAction(QIcon(":/images/icon-aeris.png"), Data::char_names.at(3));
	action->setData("{AERIS}");
	action = toolBar->addAction(QIcon(":/images/icon-red_xiii.png"), Data::char_names.at(4));
	action->setData("{RED XIII}");
	action = toolBar->addAction(QIcon(":/images/icon-yuffie.png"), Data::char_names.at(5));
	action->setData("{YUFFIE}");
	action = toolBar->addAction(QIcon(":/images/icon-cait_sith.png"), Data::char_names.at(6));
	action->setData("{CAIT SITH}");
	action = toolBar->addAction(QIcon(":/images/icon-vincent.png"), Data::char_names.at(7));
	action->setData("{VINCENT}");
	action = toolBar->addAction(QIcon(":/images/icon-cid.png"), Data::char_names.at(8));
	action->setData("{CID}");
	toolBar->addSeparator();
	action = toolBar->addAction(QIcon(":/images/icon-member_1.png"), tr("Membre 1"));
	action->setData("{MEMBER 1}");
	action = toolBar->addAction(QIcon(":/images/icon-member_2.png"), tr("Membre 2"));
	action->setData("{MEMBER 2}");
	action = toolBar->addAction(QIcon(":/images/icon-member_3.png"), tr("Membre 3"));
	action->setData("{MEMBER 3}");
	toolBar->addSeparator();

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
	action = toolBar->addAction(tr("Pauses"));
	action->setData("{PAUSE}");
	action->setMenu(menu1);

	toolBar2 = new QToolBar(this);
	toolBar2->setIconSize(QSize(16, 16));
	action = toolBar2->addAction(QIcon(":/images/icon-grey.png"), tr("Gris"));
	action->setData("{GREY}");
	action = toolBar2->addAction(QIcon(":/images/icon-blue.png"), tr("Bleu"));
	action->setData("{BLUE}");
	action = toolBar2->addAction(QIcon(":/images/icon-red.png"), tr("Rouge"));
	action->setData("{RED}");
	action = toolBar2->addAction(QIcon(":/images/icon-purple.png"), tr("Violet"));
	action->setData("{PURPLE}");
	action = toolBar2->addAction(QIcon(":/images/icon-green.png"), tr("Vert"));
	action->setData("{GREEN}");
	action = toolBar2->addAction(QIcon(":/images/icon-cyan.png"), tr("Cyan"));
	action->setData("{CYAN}");
	action = toolBar2->addAction(QIcon(":/images/icon-yellow.png"), tr("Jaune"));
	action->setData("{YELLOW}");
	action = toolBar2->addAction(QIcon(":/images/icon-white.png"), tr("Blanc"));
	action->setData("{WHITE}");
	action = toolBar2->addAction(QIcon(":/images/icon-blink.png"), tr("Clignotant"));
	action->setData("{BLINK}");
	action = toolBar2->addAction(QIcon(":/images/icon-multicolour.png"), tr("Multicolor"));
	action->setData("{MULTICOLOUR}");
	toolBar2->addSeparator();
	action = toolBar2->addAction(tr("Var1"));
	action->setData("{VAR1}");
	action = toolBar2->addAction(tr("Var2"));
	action->setData("{VAR2}");
	action = toolBar2->addAction(tr("Var3"));
	action->setData("{VAR3}");
	toolBar2->addSeparator();

	menu2 = new QMenu(this);
	action = menu2->addAction(tr("Cercle"));
	action->setData("{CIRCLE}");
	action = menu2->addAction(tr("Triangle"));
	action->setData("{TRIANGLE}");
	action = menu2->addAction(tr("Carré"));
	action->setData("{SQUARE}");
	action = menu2->addAction(tr("Croix"));
	action->setData("{CROSS}");
	action = menu2->addAction(tr("Scrolling"));
	action->setData("{SCROLLING}");
	action = menu2->addAction(tr("Caractères espacés"));
	action->setData("{SPACED CHARACTERS}");
	action = menu2->addAction(tr("Accès mémoire"));
	action->setData("{MEMORY:var[1][0];size=1}");
	action = menu2->addAction(tr("Nouvelle page²"));
	action->setData("{NEW PAGE 2}");
	action = toolBar2->addAction(tr("Autres"));
	action->setMenu(menu2);

	textEdit = new QPlainTextEdit(this);
	textEdit->setTabStopWidth(40);
	textEdit->setLineWrapMode(QPlainTextEdit::NoWrap);
	new TextHighlighter(textEdit->document());

	QToolBar *_toolBar = new QToolBar();
	_toolBar->setIconSize(QSize(14,14));
	QAction *add_A = _toolBar->addAction(QIcon(":/images/plus.png"), tr("Ajouter texte"), this, SLOT(addText()));
	add_A->setShortcut(QKeySequence("Ctrl++"));
	add_A->setStatusTip(tr("Ajouter un texte"));
	QAction *del_A = _toolBar->addAction(QIcon(":/images/minus.png"), tr("Supprimer texte"), this, SLOT(delText()));
	del_A->setShortcut(Qt::Key_Delete);
	del_A->setStatusTip(tr("Supprimer un texte"));

	QVBoxLayout *textListLayout = new QVBoxLayout;
	textListLayout->addWidget(_toolBar);
	textListLayout->addWidget(liste1);

	QVBoxLayout *toolBars = new QVBoxLayout();
	toolBars->addWidget(toolBar);
	toolBars->addWidget(toolBar2);
	toolBars->setContentsMargins(QMargins());
	toolBars->setSpacing(0);

	QWidget *groupTextPreview = new QWidget(this);
	textPreview = new TextPreview(groupTextPreview);
	textPreview->setReadOnly(true);//TODO: unlock

	prevPage = new QToolButton(groupTextPreview);
	prevPage->setArrowType(Qt::UpArrow);
	prevPage->setToolButtonStyle(Qt::ToolButtonIconOnly);
	nextPage = new QToolButton(groupTextPreview);
	nextPage->setArrowType(Qt::DownArrow);
	nextPage->setToolButtonStyle(Qt::ToolButtonIconOnly);
	textPage = new QLabel(groupTextPreview);
	textPage->setTextFormat(Qt::PlainText);

//	prevWin = new QToolButton(groupTextPreview);
//	prevWin->setArrowType(Qt::UpArrow);
//	prevWin->setToolButtonStyle(Qt::ToolButtonIconOnly);
//	nextWin = new QToolButton(groupTextPreview);
//	nextWin->setArrowType(Qt::DownArrow);
//	nextWin->setToolButtonStyle(Qt::ToolButtonIconOnly);
//	textWin = new QLabel(groupTextPreview);
//	textWin->setTextFormat(Qt::PlainText);

//	xCoord = new QSpinBox(groupTextPreview);
//	yCoord = new QSpinBox(groupTextPreview);
//	xCoord->setRange(-32768, 32767);
//	yCoord->setRange(-32768, 32767);
//	xCoord->setReadOnly(true);
//	yCoord->setReadOnly(true);

//	wSize = new QSpinBox(groupTextPreview);
//	hSize = new QSpinBox(groupTextPreview);
//	wSize->setRange(0, 65535);
//	hSize->setRange(0, 65535);
//	wSize->setReadOnly(true);
//	hSize->setReadOnly(true);

	QGridLayout *layoutTextPreview = new QGridLayout(groupTextPreview);
	layoutTextPreview->addWidget(textPreview, 0, 0, 8, 1, Qt::AlignLeft | Qt::AlignTop);
	layoutTextPreview->addWidget(prevPage, 0, 1, 1, 2, Qt::AlignLeft | Qt::AlignTop);
	layoutTextPreview->addWidget(textPage, 1, 1, 1, 2, Qt::AlignLeft | Qt::AlignTop);
	layoutTextPreview->addWidget(nextPage, 2, 1, 1, 2, Qt::AlignLeft | Qt::AlignTop);
//	layoutTextPreview->addWidget(new QLabel(tr("X")), 3, 1, Qt::AlignLeft | Qt::AlignHCenter);
//	layoutTextPreview->addWidget(xCoord, 3, 2, Qt::AlignLeft | Qt::AlignHCenter);
//	layoutTextPreview->addWidget(new QLabel(tr("Y")), 4, 1, Qt::AlignLeft | Qt::AlignHCenter);
//	layoutTextPreview->addWidget(yCoord, 4, 2, Qt::AlignLeft | Qt::AlignHCenter);
//	layoutTextPreview->addWidget(new QLabel(tr("W")), 3, 3, Qt::AlignLeft | Qt::AlignHCenter);
//	layoutTextPreview->addWidget(wSize, 3, 4, Qt::AlignLeft | Qt::AlignHCenter);
//	layoutTextPreview->addWidget(new QLabel(tr("H")), 4, 3, Qt::AlignLeft | Qt::AlignHCenter);
//	layoutTextPreview->addWidget(hSize, 4, 4, Qt::AlignLeft | Qt::AlignHCenter);
//	layoutTextPreview->addWidget(prevWin, 5, 1, 1, 2, Qt::AlignLeft | Qt::AlignBottom);
//	layoutTextPreview->addWidget(textWin, 6, 1, 1, 2, Qt::AlignLeft | Qt::AlignBottom);
//	layoutTextPreview->addWidget(nextWin, 7, 1, 1, 2, Qt::AlignLeft | Qt::AlignBottom);
	layoutTextPreview->setColumnStretch(2, 1);
	layoutTextPreview->setRowStretch(2, 1);
	layoutTextPreview->setRowStretch(5, 1);
	layoutTextPreview->setContentsMargins(QMargins());

	groupTextPreview->setFixedHeight(224);

	QGridLayout *layout = new QGridLayout();
	layout->addWidget(dispUnusedText, 0, 0, 1, 2);
	layout->addLayout(textListLayout, 1, 0, 3, 1);
	layout->addLayout(toolBars, 1, 1);
	layout->addWidget(textEdit, 2, 1);
	layout->addWidget(groupTextPreview, 3, 1);
	liste1->setFocus();
	setLayout(layout);
	adjustSize();

	connect(liste1, SIGNAL(currentItemChanged(QListWidgetItem*,QListWidgetItem*)), SLOT(selectText(QListWidgetItem*,QListWidgetItem*)));
	connect(dispUnusedText, SIGNAL(toggled(bool)), SLOT(showList()));
	connect(toolBar, SIGNAL(actionTriggered(QAction*)), SLOT(insertTag(QAction*)));
	connect(toolBar2, SIGNAL(actionTriggered(QAction*)), SLOT(insertTag(QAction*)));
	connect(menu1, SIGNAL(triggered(QAction*)), SLOT(insertTag(QAction*)));
	connect(menu2, SIGNAL(triggered(QAction*)), SLOT(insertTag(QAction*)));
	connect(textEdit, SIGNAL(textChanged()), SLOT(setTextChanged()));
	connect(textEdit, SIGNAL(cursorPositionChanged()), SLOT(emitFromChanged()));
	connect(prevPage, SIGNAL(released()), SLOT(prevTextPreviewPage()));
	connect(nextPage, SIGNAL(released()), SLOT(nextTextPreviewPage()));
//	connect(prevWin, SIGNAL(released()), SLOT(prevTextPreviewWin()));
//	connect(nextWin, SIGNAL(released()), SLOT(nextTextPreviewWin()));
}

void TextManager::emitFromChanged()
{
	emit fromChanged(textEdit->textCursor().position());
}

void TextManager::focusInEvent(QFocusEvent *)
{
	liste1->setFocus();
}

void TextManager::setField(Field *field)
{
	this->field = field;
	this->scriptsAndTexts = field->scriptsAndTexts();
//	_windows.clear();
//	_text2win.clear();
//	scriptsAndTexts->listWindows(_windows, _text2win);
	usedTexts = scriptsAndTexts->listUsedTexts();
	showList();
	liste1->setCurrentRow(0);
	selectText(liste1->item(0));
}

void TextManager::setTextChanged()
{
	QListWidgetItem *item = liste1->currentItem();
	if(item == NULL)	return;

	QString newText = textEdit->toPlainText();
	FF7Text *t = scriptsAndTexts->text(item->data(Qt::UserRole).toInt());
	bool jp = Config::value("jp_txt", false).toBool();
	if(newText != t->getText(jp)) {
		t->setText(newText, jp);
		textPreview->setText(t->getData());
		changeTextPreviewPage();
		changeTextPreviewWin();
		emit modified();
	}
}

/*QList<FF7Window> TextManager::getWindows(quint8 textID) const
{
	QList<quint64> winIDs = _text2win.values(textID);
	QList<quint64> sawAlready;
	QList<FF7Window> windows;
	foreach(const quint64 &winID, winIDs) {
		if(!sawAlready.contains(winID)) {
			windows.append(_windows.values(winID));
			sawAlready.append(winID);
		}
	}
	return windows;
}*/

void TextManager::selectText(QListWidgetItem *item, QListWidgetItem *)
{
	if(!item)	return;
	int textID = item->data(Qt::UserRole).toInt();
	FF7Text *t = scriptsAndTexts->text(textID);
//	textPreview->resetCurrentWin();
//	textPreview->setWins(getWindows(textID));
	textPreview->setText(t->getData());
	textEdit->setPlainText(t->getText(Config::value("jp_txt", false).toBool()));
	changeTextPreviewPage();
	changeTextPreviewWin();
	emit textIDChanged(textID);
}

void TextManager::showList()
{
	bool show = dispUnusedText->isChecked();
	liste1->blockSignals(true);
	int nbTextes = scriptsAndTexts->getNbTexts();
	liste1->clear();

	Config::setValue("dispUnusedText", show);

	for(int i=0 ; i<nbTextes ; ++i)
	{
		if(!show && !usedTexts.contains(i))	continue;

		QListWidgetItem *item = new QListWidgetItem(tr("Texte %1").arg(i));
		item->setData(Qt::UserRole, i);
		liste1->addItem(item);
		if(!usedTexts.contains(i)) {
			item->setForeground(Qt::darkGray);
		}
	}
	liste1->setCurrentRow(0);
	liste1->blockSignals(false);
}

void TextManager::updateText()
{
	selectText(liste1->currentItem());
	textPreview->calcSize();
}

void TextManager::gotoText(int textID, int from, int size)
{
	for(int i=0 ; i<liste1->count() ; ++i) {
		if(textID == liste1->item(i)->data(Qt::UserRole).toInt()) {
			blockSignals(true);
			textEdit->blockSignals(true);
			liste1->setCurrentItem(liste1->item(i));
			selectText(liste1->item(i));
			liste1->scrollToItem(liste1->item(i));
			QTextCursor t = textEdit->textCursor();
			t.setPosition(from);
			t.setPosition(from + size, QTextCursor::KeepAnchor);
			textEdit->setTextCursor(t);
			blockSignals(false);
			textEdit->blockSignals(false);
		}
	}
}

void TextManager::addText()
{
	QListWidgetItem *item = liste1->currentItem();
	int row = !item ? scriptsAndTexts->getNbTexts() : item->data(Qt::UserRole).toInt()+1;
	liste1->blockSignals(true);
	scriptsAndTexts->insertText(row);
	usedTexts = scriptsAndTexts->listUsedTexts();
	dispUnusedText->setChecked(true);
	showList();
	liste1->setCurrentRow(row < liste1->count() ? row : liste1->count()-1);
	emit modified();
	liste1->blockSignals(false);
}

void TextManager::delText()
{
	QListWidgetItem *item = liste1->currentItem();
	if(!item) return;
	liste1->blockSignals(true);
	int row=item->data(Qt::UserRole).toInt();
	if(usedTexts.contains(row)) {
		QMessageBox::StandardButton rep = QMessageBox::warning(this, tr("Texte utilisé dans les script"), tr("Ce texte est utilisé par un ou plusieurs scripts de cet écran.\nLe supprimer remplacera les appels à ce texte par des appels au texte qui suit.\nÊtes-vous sûr de vouloir continuer ?"), QMessageBox::Yes | QMessageBox::Cancel, QMessageBox::Cancel);
		if(rep == QMessageBox::Cancel) {
			liste1->blockSignals(false);
			return;
		}
	}
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
	int currentPage = textPreview->getCurrentPage(), nbPage = textPreview->getNbPages();

	textPage->setText(tr("Page %1/%2").arg(currentPage).arg(nbPage));
	prevPage->setEnabled(currentPage > 1);
	nextPage->setEnabled(currentPage < nbPage);
	textPage->setEnabled(nbPage > 0);
}

void TextManager::prevTextPreviewWin()
{
//	textPreview->prevWin();
//	changeTextPreviewWin();
}

void TextManager::nextTextPreviewWin()
{
//	textPreview->nextWin();
//	changeTextPreviewWin();
}

void TextManager::changeTextPreviewWin()
{
//	int currentWin = textPreview->getCurrentWin(), nbWin = textPreview->getNbWin();

//	textWin->setText(tr("Fenêtre %1/%2").arg(currentWin).arg(nbWin));
//	prevWin->setEnabled(currentWin > 1);
//	nextWin->setEnabled(currentWin < nbWin);
//	textWin->setEnabled(nbWin > 0);
//	xCoord->setEnabled(nbWin > 0);
//	yCoord->setEnabled(nbWin > 0);
//	wSize->setEnabled(nbWin > 0);
//	hSize->setEnabled(nbWin > 0);

//	updateWindowCoord();
}

void TextManager::changeRect(QRect rect)
{
	xCoord->setValue(rect.x());
	yCoord->setValue(rect.y());
	wSize->setValue(rect.width());
	hSize->setValue(rect.height());
}

void TextManager::changeXCoord(int x)
{
	if(textPreview->getNbWin()<=0 || liste1->currentItem()==NULL)	return;

	int textID = liste1->currentItem()->data(Qt::UserRole).toInt();
	int winID = textPreview->getCurrentWin()-1;
	FF7Window ff7Window = textPreview->getWindow();
	if(ff7Window.x != x) {
		ff7Window.x = x;

		qDebug() << "changeXCoord()" << x << textID << winID;

		scriptsAndTexts->setWindow(ff7Window);
//		_windows.clear();
//		_text2win.clear();
//		scriptsAndTexts->listWindows(_windows, _text2win); // refresh
//		textPreview->setWins(getWindows(textID));
		emit modified();
	}
}

void TextManager::changeYCoord(int y)
{
	if(textPreview->getNbWin()<=0 || liste1->currentItem()==NULL)	return;

	int textID = liste1->currentItem()->data(Qt::UserRole).toInt();
	int winID = textPreview->getCurrentWin()-1;
	FF7Window ff7Window = textPreview->getWindow();
	if(ff7Window.y != y) {
		ff7Window.y = y;

		qDebug() << "changeYCoord()" << y << textID << winID;

		scriptsAndTexts->setWindow(ff7Window);
//		_windows.clear();
//		_text2win.clear();
//		scriptsAndTexts->listWindows(_windows, _text2win); // refresh
//		textPreview->setWins(getWindows(textID));
		emit modified();
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
	if(sender() != action->parentWidget())	return;// toolBar/Menu signals hack
	textEdit->insertPlainText(action->data().toString());
	textEdit->setFocus();
}

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
#include "TextPreview.h"
#include "core/FF7Text.h"
#include "Data.h"
#include "core/Config.h"

QTimer TextPreview::timer;
bool TextPreview::curFrame = true;
int TextPreview::startMulticolor = DARKGREY;
int TextPreview::multicolor = -1;
int TextPreview::fontColor = WHITE;
QImage TextPreview::fontImage;

TextPreview::TextPreview(QWidget *parent)
	: QWidget(parent), _currentPage(0), _currentWin(0), acceptMove(false), readOnly(false)
{
	pagesPos.append(0);

	connect(&timer, SIGNAL(timeout()), SLOT(animate()));

	setFixedSize(320, 224);
	clear();

	if(names.isEmpty()) {
		fontImage = QImage(":/images/font.png");
		QStringList dataNames = Data::char_names;
		dataNames.replace(9, tr("Membre 1"));
		dataNames.replace(10, tr("Membre 2"));
		dataNames.replace(11, tr("Membre 3"));
//		bool jp = Config::value("jp_txt", false).toBool();
//		Config::setValue("jp_txt", false);
		for(int i=0 ; i<12 ; ++i) {
			QByteArray nameData = FF7Text(dataNames.at(i), false).getData();
			names.append(nameData);
			namesWidth[i] = calcFF7TextWidth(nameData);
		}
//		Config::setValue("jp_txt", jp);
	}
}

QPixmap TextPreview::getIconImage(int iconId)
{
	return QPixmap(":/images/keys.png").copy(iconId*16, 0, 16, 16);
}

int TextPreview::calcFF7TextWidth(const QByteArray &ff7Text)
{
	int width = 0;

	foreach(const quint8 &c, ff7Text) {
		if(c<0xe0) {
			width += charFullWidth(0, c);
		}
	}

	return width;
}

void TextPreview::clear()
{
	ff7Text.clear();
//	ff7Windows.clear();
	maxW=maxH=0;
	update();
}

void TextPreview::setReadOnly(bool ro)
{
	readOnly = ro;
}

void TextPreview::setWins(const QList<FF7Window> &windows, bool update)
{
//	qDebug() << "TextPreview::setWins()";
	ff7Windows = windows;
	if(update)
		this->update();
	//qDebug() << "type" << window.type << "x" << window.x << "y" << window.y << "u1" << window.u1;
}

void TextPreview::resetCurrentWin()
{
	_currentWin = 0;
}

int TextPreview::currentWin() const
{
	return ff7Windows.isEmpty() ? 0 : _currentWin+1;
}

FF7Window TextPreview::getWindow() const
{
	if(!ff7Windows.isEmpty())
		return ff7Windows.at(_currentWin);
	else {
		FF7Window ff7Window = FF7Window();
		ff7Window.type = NOWIN;
		return ff7Window;
	}
}

int TextPreview::winCount() const
{
	return ff7Windows.size();
}

void TextPreview::nextWin()
{
	if(_currentWin+1 < ff7Windows.size()) {
//		qDebug() << "TextPreview::nextWin()";
		++_currentWin;
		update();
	}
}

void TextPreview::prevWin()
{
	if(_currentWin > 0) {
//		qDebug() << "TextPreview::prevWin()";
		--_currentWin;
		update();
	}
}

void TextPreview::clearWin()
{
	ff7Windows.clear();
}

void TextPreview::setText(const QByteArray &textData, bool reset)
{
//	qDebug() << "TextPreview::setText()";
	ff7Text = textData;
	if(reset)
		_currentPage = 0;
	calcSize();
}

int TextPreview::currentPage() const
{
	return pagesPos.isEmpty() ? 0 : _currentPage+1;
}

int TextPreview::pageCount() const
{
	return pagesPos.size();
}

void TextPreview::nextPage()
{
	if(_currentPage+1 < pagesPos.size()) {
//		qDebug() << "TextPreview::nextPage()";
		++_currentPage;
		update();
	}
}

void TextPreview::prevPage()
{
	if(_currentPage > 0) {
//		qDebug() << "TextPreview::prevPage()";
		--_currentPage;
		update();
	}
}

void TextPreview::calcSize()
{
//	qDebug() << "TextPreview::calcSize()";

	int line=0, width=20, height=25, size=ff7Text.size();
	maxW=maxH=0;
	pagesPos.clear();
	pagesPos.append(0);
	quint8 caract;
	bool jp = Config::value("jp_txt", false).toBool(), spaced_characters=false;

	for(int i=0 ; i<size ; ++i) {
		caract = (quint8)ff7Text.at(i);
		if(caract==0xff) break;
		switch(caract) {
		case 0xe8: // New Page
		case 0xe9: // New Page 2
			if(height>maxH)	maxH = height;
			if(width>maxW)	maxW = width;
			width = 18;
			height = 22;
			pagesPos.append(i+1);
			break;
		case 0xe7: // \n
			if(width>maxW)	maxW = width;
			++line;
			width = 18;
			height += 16;
			break;
		case 0xfa: // Jap 1
			++i;
			caract = (quint8)ff7Text.at(i);
			if(jp) {
				width += spaced_characters ? 13 : charFullWidth(2, caract);
			} else if(caract < 0xd2) {
				width += spaced_characters ? 13 : 1;
			}
			break;
		case 0xfb: // Jap 2
			++i;
			if(jp) {
				caract = (quint8)ff7Text.at(i);
				width += spaced_characters ? 13 : charFullWidth(3, caract);
			}
			break;
		case 0xfc: // Jap 3
			++i;
			if(jp) {
				caract = (quint8)ff7Text.at(i);
				width += spaced_characters ? 13 : charFullWidth(4, caract);
			}
			break;
		case 0xfd: // Jap 4
			++i;
			if(jp) {
				caract = (quint8)ff7Text.at(i);
				width += spaced_characters ? 13 : charFullWidth(5, caract);
			}
			break;
		case 0xfe: // Jap 5 + add
			++i;
			if(i >= size)		break;
			caract = (quint8)ff7Text.at(i);
			if(caract == 0xdd)
				++i;
			else if(caract == 0xde || caract == 0xdf || caract == 0xe1) {
				if(caract == 0xe1)		width += spaced_characters ? 52 : 12;
				int zeroId = !jp ? 0x10 : 0x33;
				width += spaced_characters ? 13 : charFullWidth(0, zeroId);
			} else if(caract == 0xe2)
				i += 4;
			else if(caract == 0xe9)
				spaced_characters = !spaced_characters;
			else if(caract < 0xd2 && jp)
				width += spaced_characters ? 13 : charFullWidth(6, caract);
			break;
		default:
			if(!jp && caract==0xe0) {// {CHOICE}
				width += spaced_characters ? 130 : 30;
			} else if(!jp && caract==0xe1) {// \t
				width += spaced_characters ? 52 : 12;
			} else if(!jp && caract>=0xe2 && caract<=0xe4) {// duo
				const char *duo = optimisedDuo[caract-0xe2];
				width += spaced_characters ? 13 : charFullWidth(1, (quint8)duo[0]);
				width += spaced_characters ? 13 : charFullWidth(1, (quint8)duo[1]);
			} else if(caract>=0xea && caract<=0xf5) {// Character names
				width += spaced_characters ? 13*names.at(caract-0xea).size() : namesWidth[caract-0xea];
			} else if(caract>=0xf6 && caract<=0xf9) {// Keys
				width += 17;
			} else {
				if(jp) {
					width += spaced_characters ? 13 : charFullWidth(1, caract);
				} else {
					width += spaced_characters ? 13 : charFullWidth(0, caract);
				}
			}
			break;
		}
	}

	if(height>maxH)	maxH = height;
	if(width>maxW)	maxW = width;
	if(maxW>322)	maxW = 322;
	if(maxH>226)	maxH = 226;

	update();
}

QSize TextPreview::getCalculatedSize() const
{
	return QSize(maxW, maxH);
}

QPoint TextPreview::realPos(const FF7Window &ff7Window)
{
//	qDebug() << "TextPreview::realPos()";
	if(ff7Window.type == NOWIN)	return QPoint();

	int windowX=ff7Window.x, windowY=ff7Window.y;
	if(windowX+ff7Window.w>312) {
		windowX = 312-ff7Window.w;
	}
	if(windowY+ff7Window.h>223) {
		windowY = 223-ff7Window.h;
	}

	if(windowX<8)	windowX = 8;
	if(windowY<8)	windowY = 8;

	return QPoint(windowX, windowY);
}

void TextPreview::animate()
{
//	qDebug() << "TextPreview::animate()";

	curFrame = !curFrame;
	--startMulticolor;
	if(startMulticolor < 0)		startMulticolor = 7;
	repaint();
}

void TextPreview::drawWindow(QPainter *painter, WindowType type) const
{
	QRgb windowColorTopLeft = Config::value("windowColorTopLeft", qRgb(0,88,176)).toInt();
	QRgb windowColorTopRight = Config::value("windowColorTopRight", qRgb(0,0,80)).toInt();
	QRgb windowColorBottomLeft = Config::value("windowColorBottomLeft", qRgb(0,0,128)).toInt();
	QRgb windowColorBottomRight = Config::value("windowColorBottomRight", qRgb(0,0,32)).toInt();

	drawWindow(painter, maxW, maxH, windowColorTopLeft, windowColorTopRight, windowColorBottomLeft, windowColorBottomRight, type);
}

void TextPreview::drawWindow(QPainter *painter, int maxW, int maxH, QRgb colorTopLeft, QRgb colorTopRight, QRgb colorBottomLeft, QRgb colorBottomRight, WindowType type)
{
	if(type != WithoutFrame) {
		if(type == Transparent) {
			colorTopLeft = qRgba(qRed(colorTopLeft), qGreen(colorTopLeft), qBlue(colorTopLeft), 127);
			colorTopRight = qRgba(qRed(colorTopRight), qGreen(colorTopRight), qBlue(colorTopRight), 127);
			colorBottomLeft = qRgba(qRed(colorBottomLeft), qGreen(colorBottomLeft), qBlue(colorBottomLeft), 127);
			colorBottomRight = qRgba(qRed(colorBottomRight), qGreen(colorBottomRight), qBlue(colorBottomRight), 127);
		}

		// Thx Sithlord48 :3
		QImage gradient(2, 2, QImage::Format_ARGB32);
		gradient.setPixel(0, 0, colorTopLeft);
		gradient.setPixel(1, 0, colorTopRight);
		gradient.setPixel(0, 1, colorBottomLeft);
		gradient.setPixel(1, 1, colorBottomRight);
		painter->setBrush(QPixmap::fromImage(gradient.scaled(maxW, maxH, Qt::IgnoreAspectRatio, Qt::SmoothTransformation)));

		/* Frame */
		QPen pen;
		pen.setStyle(Qt::NoPen);
		painter->setPen(pen);
		painter->drawRect(3, 3, maxW-6, maxH-6);

		// Vertical

		painter->setPen(QColor(123,123,123));

		painter->drawPoint(1, 1);
		painter->drawPoint(2, 2);
		painter->drawLine(0, 2, 0, maxH-4);
		painter->drawPoint(1, maxH-3);
		painter->drawPoint(2, maxH-4);

		painter->drawPoint(maxW-2, 2);
		painter->drawLine(maxW-3, 3, maxW-3, maxH-4);
		painter->drawPoint(maxW-2, maxH-3);

		painter->setPen(QColor(198,198,198));

		painter->drawLine(1, 2, 1, maxH-4);
		painter->drawPoint(2, maxH-3);

		painter->drawPoint(maxW-3, 2);
		painter->drawLine(maxW-2, 3, maxW-2, maxH-4);
		painter->drawPoint(maxW-3, maxH-3);

		painter->setPen(QColor(49,49,49));

		painter->drawLine(2, 3, 2, maxH-5);
		painter->drawPoint(0, maxH-3);
		painter->drawPoint(1, maxH-2);

		painter->drawPoint(maxW-2, 1);
		painter->drawLine(maxW-1, 2, maxW-1, maxH-3);
		painter->drawPoint(maxW-2, maxH-2);

		// Horizontal
		painter->setPen(QColor(148,148,148));

		painter->drawLine(2, 0, maxW-4, 0);
		painter->drawPoint(maxW-3, 1);
		painter->drawPoint(maxW-4, 2);

		painter->drawPoint(2, maxH-2);
		painter->drawLine(3, maxH-3, maxW-4, maxH-3);
		painter->drawPoint(maxW-3, maxH-2);

		painter->setPen(QColor(222,222,222));

		painter->drawLine(2, 1, maxW-4, 1);
		painter->drawLine(3, maxH-2, maxW-4, maxH-2);

		painter->setPen(QColor(74,74,74));

		painter->drawLine(3, 2, maxW-5, 2);
		painter->drawPoint(maxW-3, 0);
		painter->drawLine(2, maxH-1, maxW-3, maxH-1);
	}
}

bool TextPreview::drawTextArea(QPainter *painter)
{
	bool blink = false, use_timer = false, jp = Config::value("jp_txt", false).toBool();
	FF7Window ff7Window = getWindow();
	spaced_characters = false;
	multicolor = -1;
	int savFontColor = fontColor;
	WindowType mode = Normal;

	/* Window Background */

	if(ff7Window.type!=NOWIN) {
		painter->translate(realPos(ff7Window));
		maxW = ff7Window.w;
		maxH = ff7Window.h;
		mode = (WindowType)ff7Window.mode;
	}

	drawWindow(painter, mode);

	/* Text */

	if(ff7Text.isEmpty())	return false;

	setFontColor(WHITE);

	int charId, charId2, line=0, x = 8, y = 6;
	int start = pagesPos.value(_currentPage, 0), size = ff7Text.size();

	for(int i=start ; i<size ; ++i)
	{
		charId = (quint8)ff7Text.at(i);

		if(charId==0xff || charId==0xe8 || charId==0xe9)//end | NewPage | NewPage2
			break;
		else if(charId==0xe7)//\n
		{
			++line;
			x = 8;
			y += 16;
			if(y > maxH-16)	break;
		}
		else if(charId<0xe7)
		{
			if(!jp) {
				if(charId==0xe0)//{CHOICE}
					x += spaced_characters ? 130 : 30;
				else if(charId==0xe1)//\t
					x += spaced_characters ? 52 : 12;
				else if(charId>=0xe2 && charId<=0xe4) {
					const quint8 *opti = (const quint8 *)optimisedDuo[charId-0xe2];
					letter(&x, &y, opti[0], painter, 0);
					letter(&x, &y, opti[1], painter, 0);
				} else {
					letter(&x, &y, charId, painter, 0);
				}
			} else {
				letter(&x, &y, charId, painter, 1);
			}
		}
		else if(charId>=0xea && charId<=0xf5)
		{
			word(&x, &y, names.at(charId-0xea), painter);
		}
		else if(charId>=0xf6 && charId<=0xf9)
		{
			painter->drawPixmap(x, y, getIconImage(charId-0xf6));
			x += 17;
		}
		else
		{
			++i;
			if(i >= size)	break;
			charId2 = (quint8)ff7Text.at(i);

			switch(charId) {
			case 0xfa:
				if(jp)	letter(&x, &y, charId2, painter, 2);
				else if(charId2 < 0xd2)		x += spaced_characters ? 13 : 1;
				break;
			case 0xfb:
				if(jp)	letter(&x, &y, charId2, painter, 3);
				break;
			case 0xfc:
				if(jp)	letter(&x, &y, charId2, painter, 4);
				break;
			case 0xfd:
				if(jp)	letter(&x, &y, charId2, painter, 5);
				break;
			case 0xfe:
				if(charId2 >= 0xd2 && charId2 <= 0xd9) {
					setFontColor(charId2-0xd2, blink && !curFrame);
				} else if(charId2 == 0xda) {
					use_timer = true;
					blink = !blink;
					setFontColor(fontColor, blink && !curFrame);
				} else if(charId2 == 0xdb) {
					use_timer = true;
					if(multicolor == -1) {
						savFontColor = fontColor;
						multicolor = startMulticolor;
					} else {
						multicolor = -1;
						setFontColor(savFontColor, blink && !curFrame);
					}
				} else if(charId2 == 0xdd) {
					++i;
				} else if(charId2 == 0xde || charId2 == 0xdf) {
					letter(&x, &y, !jp ? 0x10 : 0x33, painter, jp);// zero
				} else if(charId2 == 0xe1) {
					x += spaced_characters ? 52 : 12;// tab
					letter(&x, &y, !jp ? 0x10 : 0x33, painter, jp);// zero
				} else if(charId2 == 0xe2) {
					i += 4;
				} else if(charId2 == 0xe9) {
					spaced_characters = !spaced_characters;
				} else if(charId2 < 0xd2 && jp) {
					letter(&x, &y, charId2, painter, 6);
				}
				break;
			}
		}
	}

	/* Ask */
//	if(ff7Window.type==0x48 && ff7Window.ask_last >= ff7Window.ask_first) {
//		painter->drawPixmap(10, 11+16*ff7Window.ask_first, QPixmap(":/images/cursor.png"));
//	}

	return use_timer;
}

void TextPreview::paintEvent(QPaintEvent */* event */)
{
//	qDebug() << "TextPreview::paintEvent()";

	bool use_timer = false;
	QPixmap pix(width(), height());
	QPainter painter(&pix);

	/* Screen background */

	painter.setBrush(Qt::black);
	painter.drawRect(0, 0, width(), height());

	use_timer = drawTextArea(&painter);

	painter.end();

	QPainter painter2(this);

	if(!isEnabled()) {
		QStyleOption opt;
		opt.initFrom(this);
		painter2.drawPixmap(0, 0, QWidget::style()->generatedIconPixmap(QIcon::Disabled, pix, &opt));
	}
	else {
		painter2.drawPixmap(0, 0, pix);
	}

	painter2.end();

	if(!timer.isActive()) {
		if(use_timer)	timer.start(100);
	}
	else {
		if(!use_timer)	timer.stop();
	}
}

void TextPreview::mousePressEvent(QMouseEvent *event)
{
	if(event->button() == Qt::LeftButton && !readOnly) {
		FF7Window ff7Window = getWindow();
		if(ff7Window.type!=NOWIN) {
			QPoint real = realPos(ff7Window);

			acceptMove = event->x() >= real.x() && event->x() < real.x()+maxW
						 && event->y() >= real.y() && event->y() < real.y()+maxH;

			if(acceptMove) {
				moveStartPosition = event->pos();
				setCursor(Qt::ClosedHandCursor);
			}
		}
		else
			acceptMove = false;
	}
}

void TextPreview::mouseMoveEvent(QMouseEvent *event)
{
	if(!(event->buttons() & Qt::LeftButton) && !acceptMove)
		return;

	FF7Window ff7Window = getWindow();

	if(ff7Window.type==NOWIN)	return;

	int x = ff7Window.x + event->x() - moveStartPosition.x();
	int y = ff7Window.y + event->y() - moveStartPosition.y();

	if(x<0)	x = 0;
	if(y<0)	y = 0;

	ff7Window.x = x;
	ff7Window.y = y;
	QPoint real = realPos(ff7Window);
	ff7Window.x = real.x();
	ff7Window.y = real.y();

	emit positionChanged(real);

	moveStartPosition = event->pos();

	ff7Windows.replace(_currentWin, ff7Window);

	update();
}

void TextPreview::mouseReleaseEvent(QMouseEvent *)
{
	if(acceptMove) {
		unsetCursor();
	}
}

void TextPreview::letter(int *x, int *y, int charId, QPainter *painter, quint8 tableId)
{
	int charWidth = charW(tableId, charId);
	int leftPadd = leftPadding(tableId, charId);

	if(*x + leftPadd + charWidth > maxW) {
		*x = 8;
		*y += 16;
	}

	if(multicolor != -1) {
		setFontColor(multicolor);
		multicolor = (multicolor + 1) % 8;
	}

	if(!spaced_characters)	*x += leftPadd;
	painter->drawImage(*x, *y, letterImage(tableId, charId));
	*x += spaced_characters ? 13 : charWidth;
}

void TextPreview::word(int *x, int *y, const QByteArray &charIds, QPainter *painter, quint8 tableId)
{
	foreach(char charId, charIds) {
		letter(x, y, charId, painter, tableId);
	}
}

quint8 TextPreview::charW(int tableId, int charId)
{
	return Data::windowBin.isValid()
			? Data::windowBin.charWidth(tableId, charId)
			: CHAR_WIDTH(charWidth[tableId][charId]);
}

quint8 TextPreview::leftPadding(int tableId, int charId)
{
	return Data::windowBin.isValid()
			? Data::windowBin.charLeftPadding(tableId, charId)
			: LEFT_PADD(charWidth[tableId][charId]);
}

quint8 TextPreview::charFullWidth(int tableId, int charId)
{
	return charW(tableId, charId) + leftPadding(tableId, charId);
}

QImage TextPreview::letterImage(int tableId, int charId)
{
	if(Data::windowBin.isValid() && tableId == 0) {
		return Data::windowBin.letter(tableId, charId);
	} else {
		int charIdImage = charId + posTable[tableId];
		return fontImage.copy((charIdImage%21)*12, (charIdImage/21)*12, 12, 12);
	}
}

void TextPreview::setFontColor(int id, bool blink)
{
	if(Data::windowBin.isValid()) {
		Data::windowBin.setFontColor(WindowBinFile::FontColor(blink ? DARKGREY : id));
	} else {
		fontImage.setColorTable(fontPalettes[blink ? DARKGREY : id]);
	}
	fontColor = id;
}

QVector<QRgb> TextPreview::fontPalettes[8] = {
	// darkgrey
	(QVector<QRgb>() << qRgb(0x6a,0x6a,0x6a) << qRgb(0x08,0x08,0x08) << qRgba(0,0,0,0)),
	// darkblue
	(QVector<QRgb>() << qRgb(0x00,0x62,0xbd) << qRgb(0x00,0x08,0x18) << qRgba(0,0,0,0)),
	// red
	(QVector<QRgb>() << qRgb(0xbd,0x00,0x00) << qRgb(0x18,0x00,0x00) << qRgba(0,0,0,0)),
	// purple
	(QVector<QRgb>() << qRgb(0xe6,0x00,0xe6) << qRgb(0x20,0x00,0x20) << qRgba(0,0,0,0)),
	// green
	(QVector<QRgb>() << qRgb(0x5a,0xe6,0x7b) << qRgb(0x08,0x20,0x10) << qRgba(0,0,0,0)),
	// cyan
	(QVector<QRgb>() << qRgb(0x00,0xe6,0xe6) << qRgb(0x00,0x20,0x20) << qRgba(0,0,0,0)),
	// yellow
	(QVector<QRgb>() << qRgb(0xe6,0xe6,0x00) << qRgb(0x20,0x20,0x00) << qRgba(0,0,0,0)),
	// white
	(QVector<QRgb>() << qRgb(0xe6,0xe6,0xe6) << qRgb(0x20,0x20,0x20) << qRgba(0,0,0,0))
};

quint16 TextPreview::posTable[7] =
{
	0,/* +(21*11) */
	231,/* +(21*11) */
	462,/* +(21*10) */
	672,/* +(21*11) */
	903,/* +(21*10) */
	1113,/* +(21*10) */
	1323/* (21*10) */
};

quint8 TextPreview::charWidth[7][256] =
{
	{ // International
		3, 3, 72, 10, 7, 10, 9, 3, 72, 72, 7, 7, 39, 5, 38, 6,
		8, 71, 8, 8, 8, 8, 8, 8, 8, 8, 69, 4, 7, 8, 7, 6,
		10, 9, 7, 8, 8, 7, 7, 8, 8, 3, 6, 7, 7, 11, 8, 9,
		7, 9, 7, 7, 7, 8, 9, 11, 8, 9, 7, 4, 6, 4, 7, 8,
		4, 7, 7, 6, 7, 7, 6, 7, 7, 3, 4, 6, 3, 11, 7, 7,
		7, 7, 5, 6, 6, 7, 7, 11, 7, 7, 6, 5, 3, 5, 8, 68,
		73, 76, 72, 73, 73, 9, 8, 7, 7, 7, 7, 7, 7, 7, 7, 7,
		7, 7, 4, 4, 4, 4, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
		11, 6, 7, 8, 11, 6, 7, 7, 9, 9, 11, 4, 5, 8, 12, 9,
		11, 7, 7, 7, 9, 7, 7, 7, 9, 8, 4, 6, 6, 9, 11, 7,
		6, 3, 8, 7, 8, 8, 9, 7, 7, 9, 1, 9, 9, 9, 12, 11,
		8, 12, 6, 6, 4, 4, 7, 7, 7, 9, 7, 9, 5, 5, 7, 7,
		8, 3, 4, 6, 13, 9, 7, 9, 7, 7, 3, 4, 4, 3, 9, 9,
		8, 9, 8, 8, 8, 3, 6, 7, 5, 6, 3, 6, 5, 6, 5, 5,
		1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
	},{ // Jap - 0
		13, 13, 11, 13, 12, 13, 12, 12, 12, 13, 12, 12, 11, 12, 12, 10,
		12, 12, 12, 11, 13, 11, 12, 9, 12, 12, 12, 13, 10, 12, 12, 12,
		12, 12, 12, 12, 12, 12, 8, 11, 12, 13, 12, 10, 12, 12, 12, 12,
		12, 11, 13, 8, 70, 8, 8, 9, 8, 8, 7, 8, 8, 39, 41, 8,
		12, 11, 9, 11, 10, 12, 12, 12, 11, 11, 10, 12, 10, 10, 10, 8,
		11, 11, 10, 9, 11, 10, 11, 9, 10, 11, 11, 11, 9, 10, 10, 11,
		11, 10, 10, 11, 11, 10, 6, 9, 10, 10, 11, 12, 10, 11, 11, 11,
		11, 12, 10, 12, 11, 10, 9, 12, 11, 12, 9, 11, 10, 12, 9, 12,
		11, 11, 10, 11, 11, 10, 10, 9, 8, 8, 11, 10, 8, 11, 9, 10,
		11, 11, 11, 11, 9, 10, 10, 12, 10, 11, 9, 10, 9, 8, 9, 9,
		9, 9, 7, 8, 8, 9, 8, 8, 7, 8, 9, 8, 9, 10, 69, 41,
		73, 74, 4, 9, 10, 9, 9, 9, 9, 9, 9, 9, 69, 7, 8, 9,
		11, 9, 9, 9, 10, 9, 9, 11, 9, 9, 11, 9, 11, 10, 70, 9,
		9, 13, 12, 13, 13, 70, 10, 6, 6, 11, 13, 10, 9, 72, 72, 72,
		72, 9, 9, 1, 1, 1, 13, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
	},{ // Jap - 1
		13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13,
		13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 10,
		13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13,
		13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13,
		13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13,
		13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13,
		13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13,
		13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13,
		13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13,
		13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 12, 13, 13,
		13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13,
		13, 13, 13, 13, 13, 13, 13, 13, 13, 12, 13, 13, 13, 13, 13, 11,
		13, 12, 10, 13, 13, 12, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13,
		13, 13, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
	},{ // Jap - 2
		13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13,
		13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13,
		13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13,
		13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13,
		13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 12, 13, 13,
		13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 12, 13, 13, 13, 13,
		13, 13, 11, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13,
		13, 13, 13, 13, 9, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13,
		13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13,
		13, 13, 13, 13, 13, 11, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13,
		13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13,
		13, 13, 13, 13, 13, 13, 13, 13, 13, 12, 13, 12, 13, 13, 13, 13,
		13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13,
		13, 13, 13, 13, 11, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13,
		13, 13, 13, 13, 13, 13, 13, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
	},{ // Jap - 3
		12, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13,
		13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13,
		13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13,
		10, 13, 13, 12, 13, 13, 13, 13, 13, 12, 13, 13, 13, 13, 13, 13,
		13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13,
		13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13,
		13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13,
		13, 13, 13, 13, 12, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13,
		13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13,
		13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13,
		13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13,
		13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13,
		13, 12, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13,
		13, 13, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
	},{ // Jap - 4
		13, 13, 13, 13, 13, 13, 13, 13, 11, 13, 13, 13, 13, 13, 13, 13,
		13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13,
		13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13,
		13, 13, 13, 13, 13, 13, 13, 11, 13, 13, 13, 13, 13, 13, 13, 13,
		13, 13, 13, 13, 13, 12, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13,
		13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13,
		13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 12, 13,
		13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13,
		13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13,
		13, 13, 13, 13, 13, 13, 12, 13, 13, 13, 13, 13, 13, 13, 13, 13,
		13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13,
		13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13,
		13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13,
		13, 13, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
	},{ // Jap - 5
		13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13,
		13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13,
		13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 11, 13,
		13, 13, 13, 13, 13, 13, 13, 13, 12, 13, 13, 13, 13, 13, 13, 13,
		13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13,
		13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13,
		13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 12, 13,
		13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13,
		13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13,
		13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13,
		13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13,
		13, 13, 13, 11, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13,
		13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13,
		13, 13, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
	}
};

const char *TextPreview::optimisedDuo[3] =
{
	"\x0c\x00",//', '
	"\x0e\x02",//'."'
	"\xa9\x02" //'..."'
};

QList<QByteArray> TextPreview::names;

int TextPreview::namesWidth[12];

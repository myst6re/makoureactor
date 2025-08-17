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
#include "TextPreview.h"
#include "Data.h"
#include "core/Config.h"
#include "core/FF7Font.h"

#include <FF7Text>
#include <FF7Char>
#include <ff7tkInfo>

QTimer TextPreview::timer;
int TextPreview::curFrame10 = 0;
bool TextPreview::curFrame = true;
int TextPreview::startMulticolor = int(WindowBinFile::DarkGrey);
int TextPreview::multicolor = -1;
WindowBinFile::FontColor TextPreview::fontColor = WindowBinFile::White;
QImage TextPreview::fontImage;

TextPreview::TextPreview(QWidget *parent) :
      QWidget(parent), _currentPage(0), _currentWin(0), maxW(0), maxH(0),
      acceptMove(false), readOnly(false)
{
	pagesPos.append(0);

	connect(&timer, &QTimer::timeout, this, &TextPreview::animate);

	setFixedSize(320, 224);

	if (names.isEmpty()) {
		fontImage = QImage(":/images/font.png");
		fillNames();
	}
}

void TextPreview::fillNames()
{
	FF7Text::setJapanese(Config::value("jp_txt", false).toBool());
	QString lang = Config::value("jp_txt", false).toBool() ? QStringLiteral("ja") : QStringLiteral("en");
	QTranslator *translator = ff7tkInfo::translations().value(lang);
	for (int i = 0; i < 12; ++i) {
		if (i < 9) {
			QString defaultName = FF7Char::defaultName(i);
			if (translator != nullptr) {
				QString translatedName = translator->translate("FF7Char", defaultName.toLatin1());
				if (!translatedName.isEmpty()) {
					defaultName = translatedName;
				}
			}
			names.append(FF7Text::toFF7(Config::value(QStringLiteral("customCharName%1").arg(i), defaultName).toString()));
		} else {
			names.append(FF7Text::toFF7(tr("Member %1").arg(QString::number(i - 8))));
		}
	}
}

void TextPreview::updateNames()
{
	names.clear();
	fillNames();
}

QPixmap TextPreview::getIconImage(int iconId)
{
	return QPixmap(":/images/keys.png").copy(iconId*16, 0, 16, 16);
}

void TextPreview::clear()
{
	ff7Text.clear();
//	ff7Windows.clear();
	maxW = maxH = 0;
	update();
}

void TextPreview::setReadOnly(bool ro)
{
	readOnly = ro;
}

void TextPreview::setWins(const QList<FF7Window> &windows, bool update)
{
	ff7Windows = windows;
	if (update) {
		this->update();
	}
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
	if (!ff7Windows.isEmpty()) {
		return ff7Windows.at(_currentWin);
	}

	FF7Window ff7Window = FF7Window();
	ff7Window.type = NOWIN;

	return ff7Window;
}

bool TextPreview::setWindow(const FF7Window &win)
{
	if (!ff7Windows.isEmpty()) {
		ff7Windows[_currentWin] = win;

		return true;
	}

	return false;
}

qsizetype TextPreview::winCount() const
{
	return ff7Windows.size();
}

void TextPreview::nextWin()
{
	if (_currentWin + 1 < ff7Windows.size()) {
		++_currentWin;
		update();
	}
}

void TextPreview::prevWin()
{
	if (_currentWin > 0) {
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
	ff7Text = textData;
	if (reset) {
		_currentPage = 0;
	}
	calcSize();
}

int TextPreview::currentPage() const
{
	return pagesPos.isEmpty() ? 0 : _currentPage+1;
}

qsizetype TextPreview::pageCount() const
{
	return pagesPos.size();
}

void TextPreview::nextPage()
{
	if (_currentPage + 1 < pagesPos.size()) {
		++_currentPage;
		update();
	}
}

void TextPreview::prevPage()
{
	if (_currentPage > 0) {
		--_currentPage;
		update();
	}
}

void TextPreview::calcSize()
{
	QSize size = FF7Font::calcSize(ff7Text, pagesPos);
	maxW = size.width();
	maxH = size.height();

	update();
}

QSize TextPreview::getCalculatedSize() const
{
	return QSize(maxW, maxH);
}

void TextPreview::animate()
{
	curFrame = !curFrame;
	curFrame10 = (curFrame10 + 1) % 10;
	--startMulticolor;
	if (startMulticolor < 0) {
		startMulticolor = 7;
	}
	repaint();
}

void TextPreview::drawWindow(QPainter *painter, WindowType type) const
{
	drawWindow(painter, maxW, maxH, type);
}

void TextPreview::drawWindow(QPainter *painter, int maxW, int maxH, WindowType type)
{
	QRgb windowColorTopLeft = Config::value("windowColorTopLeft", qRgb(0,88,176)).toUInt();
	QRgb windowColorTopRight = Config::value("windowColorTopRight", qRgb(0,0,80)).toUInt();
	QRgb windowColorBottomLeft = Config::value("windowColorBottomLeft", qRgb(0,0,128)).toUInt();
	QRgb windowColorBottomRight = Config::value("windowColorBottomRight", qRgb(0,0,32)).toUInt();

	drawWindow(painter, maxW, maxH, windowColorTopLeft, windowColorTopRight, windowColorBottomLeft, windowColorBottomRight, type);
}

void TextPreview::drawWindow(QPainter *painter, int maxW, int maxH, QRgb colorTopLeft, QRgb colorTopRight, QRgb colorBottomLeft, QRgb colorBottomRight, WindowType type)
{
	if (type != WithoutFrameAndBg) {
		if (type == Transparent) {
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

		if (type == WithoutFrame) {
			painter->drawRect(0, 0, maxW, maxH);
			return;
		}

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
	bool blink = false, useTimer = false,
	     jp = Config::value("jp_txt", false).toBool();
	FF7Window ff7Window = getWindow();
	spaced_characters = false;
	multicolor = -1;
	WindowBinFile::FontColor savFontColor = fontColor;
	WindowType mode = Normal;
	int spacedCharsW = Config::value("spacedCharactersWidth", 13).toInt(),
	    choiceW = Config::value("choiceWidth", 10).toInt(),
	    tabW = Config::value("tabWidth", 4).toInt();

	/* Window Background */

	if (ff7Window.type == OpcodeKey::MPNAM) {
		painter->translate(0, 10);
		drawWindow(painter, 260, 204, Normal);
		painter->translate(320 - 156 / 2, -10);
		drawWindow(painter, 156 / 2, 156, Normal);
		painter->translate(162 - 320 + 156 / 2, 199);
		maxW = 156;
		maxH = 25;
		drawWindow(painter, maxW / 2, maxW, Normal);
	} else if (ff7Window.type != NOWIN) {
		painter->translate(ff7Window.realPos());
		maxW = ff7Window.w;
		maxH = ff7Window.h;
		mode = WindowType(ff7Window.mode);
	} else if (WindowType(ff7Window.mode) == WithoutFrameAndBg) {
		mode = WindowType(ff7Window.mode);
	}

	drawWindow(painter, mode);

	/* Text */
	setFontColor(WindowBinFile::White);

	int x = 8, y = 6;
	int start = pagesPos.value(_currentPage, 0);
	qsizetype size = ff7Text.size();

	for (int i = start; i < size; ++i) {
		quint8 charId = quint8(ff7Text.at(i));

		if (charId==0xff || charId==0xe8 || charId==0xe9) { //end | NewPage | NewPage2
			break;
		} else if (charId==0xe7) { //\n
			x = 8;
			y += 16;
			if (y > maxH-16)	break;
		} else if (charId<0xe7) {
			if (!jp) {
				if (charId==0xe0)//{CHOICE}
					x += spaced_characters ? spacedCharsW * choiceW : 3 * choiceW;
				else if (charId==0xe1)//\t
					x += spaced_characters ? spacedCharsW * tabW : 3 * tabW;
				else if (charId>=0xe2 && charId<=0xe4) {
					const char *opti = FF7Font::optimisedDuo[charId-0xe2];
					letter(&x, &y, quint8(opti[0]), painter, 0);
					letter(&x, &y, quint8(opti[1]), painter, 0);
				} else {
					letter(&x, &y, charId, painter, 0);
				}
			} else {
				letter(&x, &y, charId, painter, 1);
			}
		} else if (charId>=0xea && charId<=0xf5) {
			word(&x, &y, names.at(charId-0xea), painter, jp ? 1 : 0);
		} else if (charId>=0xf6 && charId<=0xf9) {
			painter->drawPixmap(x, y - 2, getIconImage(charId-0xf6));
			x += 17;
		} else {
			++i;
			if (i >= size)	break;
			quint8 charId2 = quint8(ff7Text.at(i));

			switch (charId) {
			case 0xfa:
				if (jp)	letter(&x, &y, charId2, painter, 2);
				else if (charId2 < 0xd2)		x += spaced_characters ? spacedCharsW : 1;
				break;
			case 0xfb:
				if (jp)	letter(&x, &y, charId2, painter, 3);
				break;
			case 0xfc:
				if (jp)	letter(&x, &y, charId2, painter, 4);
				break;
			case 0xfd:
				if (jp)	letter(&x, &y, charId2, painter, 5);
				break;
			case 0xfe:
				if (charId2 >= 0xd2 && charId2 <= 0xd9) {
					setFontColor(WindowBinFile::FontColor(charId2 - 0xd2), blink && !curFrame);
				} else if (charId2 == 0xda) {
					useTimer = true;
					blink = !blink;
					setFontColor(fontColor, blink && !curFrame);
				} else if (charId2 == 0xdb) {
					useTimer = true;
					if (multicolor == -1) {
						savFontColor = fontColor;
						multicolor = startMulticolor;
					} else {
						multicolor = -1;
						setFontColor(savFontColor, blink && !curFrame);
					}
				} else if (charId2 == 0xdd) {
					++i;
				} else if (charId2 == 0xde || charId2 == 0xdf) {
					letter(&x, &y, !jp ? 0x10 : 0x33, painter, jp);// zero
				} else if (charId2 == 0xe1) {
					x += spaced_characters ? spacedCharsW * tabW : 3 * tabW;// tab
					letter(&x, &y, !jp ? 0x10 : 0x33, painter, jp);// zero
				} else if (charId2 == 0xe2) {
					i += 4;
				} else if (charId2 == 0xe9) {
					spaced_characters = !spaced_characters;
				} else if (charId2 < 0xd2 && jp) {
					letter(&x, &y, charId2, painter, 6);
				}
				break;
			}
		}
	}

	/* Ask */
	if (ff7Window.type == OpcodeKey::ASK && _currentPage == pagesPos.size() - 1) {
		QPixmap cursor(":/images/cursor.png");
		for (int i = ff7Window.ask_first; i <= ff7Window.ask_last; ++i) {
			painter->drawPixmap(10, 6 + 16 * i, cursor);
		}
	}
	
	if (ff7Window.displayType == 0x01 || ff7Window.displayType == 0x02) {
		const quint8 dispX = ff7Window.displayX, dispY = ff7Window.displayY;
		QPixmap clock(":/images/texture-purple.png");
		// 00
		painter->drawPixmap(dispX + 0, dispY, clock, 16, 0, 16, clock.height()); // 0
		painter->drawPixmap(dispX + 16, dispY, clock, 16, 0, 16, clock.height()); // 0
		if (ff7Window.displayType == 0x01) {
			// :00
			painter->drawPixmap(dispX + 32, dispY, clock, curFrame10 < 5 ? 0 : 8, 0, 8, clock.height()); // :
			painter->drawPixmap(dispX + 40, dispY, clock, 16, 0, 16, clock.height()); // 0
			painter->drawPixmap(dispX + 56, dispY, clock, 16, 0, 16, clock.height()); // 0
		}

		useTimer = true;
	}

	return useTimer;
}

void TextPreview::paintEvent(QPaintEvent *event)
{
	Q_UNUSED(event)

	QPixmap pix(width(), height());
	QPainter painter(&pix);

	/* Screen background */

	painter.setBrush(Qt::black);
	painter.drawRect(0, 0, width(), height());

	bool useTimer = drawTextArea(&painter);

	painter.end();

	QPainter painter2(this);

	if (!isEnabled()) {
		QStyleOption opt;
		opt.initFrom(this);
		painter2.drawPixmap(0, 0, QWidget::style()->generatedIconPixmap(QIcon::Disabled, pix, &opt));
	}
	else {
		painter2.drawPixmap(0, 0, pix);
	}

	painter2.end();

	if (!timer.isActive()) {
		if (useTimer) {
			timer.start(100);
		}
	}
	else if (!useTimer) {
		timer.stop();
	}
}

void TextPreview::mousePressEvent(QMouseEvent *event)
{
	if (event->button() == Qt::LeftButton && !readOnly) {
		FF7Window ff7Window = getWindow();
		if (ff7Window.type != NOWIN && ff7Window.type != OpcodeKey::MPNAM) {
			QPoint real = ff7Window.realPos();

			acceptMove = event->position().x() >= real.x() && event->position().x() < real.x() + maxW
			        && event->position().y() >= real.y() && event->position().y() < real.y() + maxH;

			if (acceptMove) {
				moveStartPosition = event->pos();
				setCursor(Qt::ClosedHandCursor);
			}
		}
		else {
			acceptMove = false;
		}
	}
}

void TextPreview::mouseMoveEvent(QMouseEvent *event)
{
	if (!(event->buttons() & Qt::LeftButton) || !acceptMove) {
		return;
	}

	FF7Window ff7Window = getWindow();

	if (ff7Window.type == NOWIN || ff7Window.type == OpcodeKey::MPNAM) {
		return;
	}

	int x = ff7Window.x + event->position().x() - moveStartPosition.x();
	int y = ff7Window.y + event->position().y() - moveStartPosition.y();

	if (x < 0) {
		x = 0;
	}
	if (y < 0) {
		y = 0;
	}

	ff7Window.x = qint16(x);
	ff7Window.y = qint16(y);
	QPoint real = ff7Window.realPos();
	ff7Window.x = qint16(real.x());
	ff7Window.y = qint16(real.y());

	emit positionChanged(real);

	moveStartPosition = event->pos();

	ff7Windows.replace(_currentWin, ff7Window);

	update();
}

void TextPreview::mouseReleaseEvent(QMouseEvent *)
{
	if (acceptMove) {
		unsetCursor();
	}
}

void TextPreview::wheelEvent(QWheelEvent *event)
{
	int numDegrees = event->angleDelta().y() / 8,
			numSteps = numDegrees / 15;
	
	if (numDegrees != 0) {
		int oldPage = _currentPage;

		if (numSteps > 0) {
			prevPage();
		} else if (numSteps < 0) {
			nextPage();
		}

		if (oldPage != _currentPage) {
			emit pageChanged(_currentPage);
		}
	}
}

void TextPreview::letter(int *x, int *y, quint8 charId, QPainter *painter, quint8 tableId)
{
	int charWidth = FF7Font::charW(tableId, charId);
	int leftPadd = FF7Font::leftPadding(tableId, charId);

	if (*x + leftPadd + charWidth > maxW) {
		*x = 8;
		*y += 16;

		if (*y > maxH - 16) {
			return;
		}
	}

	if (multicolor != -1) {
		setFontColor(WindowBinFile::FontColor(multicolor));
		multicolor = (multicolor + 1) % 8;
	}

	if (!spaced_characters)	*x += leftPadd;
	painter->drawImage(*x, *y, letterImage(tableId, charId));
	*x += spaced_characters ? Config::value("spacedCharactersWidth", 13).toInt() : charWidth;
}

void TextPreview::word(int *x, int *y, const QByteArray &charIds, QPainter *painter, quint8 tableId)
{
	for (char charId : charIds) {
		letter(x, y, quint8(charId), painter, tableId);
	}
}

QImage TextPreview::letterImage(quint8 tableId, quint8 charId)
{
	if (Data::windowBin.isValid() &&
			(tableId != 0 || !Data::windowBin.isJp())) {
		return Data::windowBin.letter(tableId == 0 ? 0 : tableId - 1, charId, fontColor);
	} else {
		int charIdImage = charId + posTable[tableId];
		return fontImage.copy((charIdImage%21)*12, (charIdImage/21)*12, 12, 12);
	}
}

void TextPreview::setFontColor(WindowBinFile::FontColor color, bool blink)
{
	if (!Data::windowBin.isValid()) {
		fontImage.setColorTable(fontPalettes[blink ? WindowBinFile::DarkGrey : color]);
	}
	fontColor = color;
}

QList<QRgb> TextPreview::fontPalettes[8] = {
	// darkgrey
	(QList<QRgb>() << qRgb(0x6a,0x6a,0x6a) << qRgb(0x08,0x08,0x08) << qRgba(0,0,0,0)),
	// darkblue
	(QList<QRgb>() << qRgb(0x00,0x62,0xbd) << qRgb(0x00,0x08,0x18) << qRgba(0,0,0,0)),
	// red
	(QList<QRgb>() << qRgb(0xbd,0x00,0x00) << qRgb(0x18,0x00,0x00) << qRgba(0,0,0,0)),
	// purple
	(QList<QRgb>() << qRgb(0xe6,0x00,0xe6) << qRgb(0x20,0x00,0x20) << qRgba(0,0,0,0)),
	// green
	(QList<QRgb>() << qRgb(0x5a,0xe6,0x7b) << qRgb(0x08,0x20,0x10) << qRgba(0,0,0,0)),
	// cyan
	(QList<QRgb>() << qRgb(0x00,0xe6,0xe6) << qRgb(0x00,0x20,0x20) << qRgba(0,0,0,0)),
	// yellow
	(QList<QRgb>() << qRgb(0xe6,0xe6,0x00) << qRgb(0x20,0x20,0x00) << qRgba(0,0,0,0)),
	// white
	(QList<QRgb>() << qRgb(0xe6,0xe6,0xe6) << qRgb(0x20,0x20,0x20) << qRgba(0,0,0,0))
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

QList<QByteArray> TextPreview::names;

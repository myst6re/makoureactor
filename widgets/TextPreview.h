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
#ifndef TEXTPREVIEW_H
#define TEXTPREVIEW_H

#include <QtWidgets>
#include "core/field/Opcode.h"

#define NOWIN		255
#define DARKGREY	0
#define DARKBLUE	1
#define RED			2
#define PURPLE		3
#define GREEN		4
#define CYAN		5
#define YELLOW		6
#define WHITE		7

class TextPreview : public QWidget
{
	Q_OBJECT
public:
	enum WindowType {
		Normal=0, WithoutFrameAndBg, Transparent, WithoutFrame
	};

	explicit TextPreview(QWidget *parent=0);
	static void updateNames();
	void clear();
	void setReadOnly(bool ro);
	void setWins(const QList<FF7Window> &windows, bool update=true);
	void resetCurrentWin();
	int currentWin() const;
	FF7Window getWindow() const;
	bool setWindow(const FF7Window &win);
	int winCount() const;
	void nextWin();
	void prevWin();
	void clearWin();
	void setText(const QByteArray &textData, bool reset=true);
	int currentPage() const;
	int pageCount() const;
	void nextPage();
	void prevPage();
	void calcSize();
	static QSize calcSize(const QByteArray &ff7Text);
	static QSize calcSize(const QByteArray &ff7Text, QList<int> &pagesPos);
	static QPoint realPos(const FF7Window &ff7Window);
	QSize getCalculatedSize() const;
	static QPixmap getIconImage(int iconId);
	void drawWindow(QPainter *painter, WindowType type=Normal) const;
	static void drawWindow(QPainter *painter, int maxW, int maxH, WindowType type=Normal);
	static void drawWindow(QPainter *painter, int maxW, int maxH, QRgb colorTopLeft, QRgb colorTopRight, QRgb colorBottomLeft, QRgb colorBottomRight, WindowType type=Normal);
private slots:
	void animate();
signals:
	void positionChanged(const QPoint &);
	void pageChanged(int);
private:
	static void fillNames();
	bool drawTextArea(QPainter *painter);
	QList<FF7Window> ff7Windows;
	QByteArray ff7Text;
	int _currentPage;
	int _currentWin;
	QList<int> pagesPos;
	int maxW, maxH;
	static bool curFrame;
	bool acceptMove;
	bool spaced_characters;
	QPoint moveStartPosition;
	bool readOnly;

	static int startMulticolor;
	static int multicolor;
	static int fontColor;
	static QImage fontImage;
	void letter(int *x, int *y, int charId, QPainter *painter, quint8 tableId=0);
	void word(int *x, int *y, const QByteArray &charIds, QPainter *painter, quint8 tableId=0);
	static quint8 charW(int tableId, int charId);
	static quint8 leftPadding(int tableId, int charId);
	static quint8 charFullWidth(int tableId, int charId);
	static QImage letterImage(int tableId, int charId);
	static void setFontColor(int id, bool blink=false);
	static QVector<QRgb> fontPalettes[8];
	static QTimer timer;
	static int calcFF7TextWidth(const QByteArray &ff7Text);
	static quint16 posTable[7];
	static quint8 charWidth[7][256];
	static const char *optimisedDuo[3];
	static QList<QByteArray> names;
	static int biggestCharWidth;
protected:
	void paintEvent(QPaintEvent *event);
	void mousePressEvent(QMouseEvent *event);
	void mouseMoveEvent(QMouseEvent *event);
	void mouseReleaseEvent(QMouseEvent *event);
	void wheelEvent(QWheelEvent *event);
};

#endif // TEXTPREVIEW_H

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

#include <QtGui>
#include "FF7Text.h"
#include "Script.h"

#define NOWIN		255
#define DARKGREY	0
#define DARKBLUE	1
#define RED			2
#define PURPLE		3
#define GREEN		4
#define CYAN		5
#define YELLOW		6
#define WHITE		7
#define LEFT_PADD(w)	(w >> 5)
#define CHAR_WIDTH(w)	(w & 0x1F)
#include "Data.h"

class TextPreview : public QWidget
{
	Q_OBJECT
public:
	explicit TextPreview(QWidget *parent=0);
	void clear();
	void setReadOnly(bool ro);
	void setWins(const QList<FF7Window> &windows, bool update=true);
	void resetCurrentWin();
	int getCurrentWin();
	FF7Window getWindow();
	int getNbWin();
	void nextWin();
	void prevWin();
	void clearWin();
	void setText(const QByteArray &textData, bool reset=true);
	int getCurrentPage();
	int getNbPages();
	void nextPage();
	void prevPage();
	void calcSize();
	QSize getCalculatedSize() const;
	QPixmap getIconImage(int iconId);
	static void drawWindow(QPainter *painter, int maxW, int maxH, QRgb colorTopLeft, QRgb colorTopRight, QRgb colorBottomLeft, QRgb colorBottomRight);
private slots:
	void animate();
signals:
	void positionChanged(QPoint);
private:
	bool drawTextArea(QPainter *painter);
	QPoint realPos(const FF7Window &ff7Window);
	QList<FF7Window> ff7Windows;
	QByteArray ff7Text;
	int currentPage;
	int currentWin;
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
	static void setFontColor(int id, bool blink=false);
	static QVector<QRgb> fontPalettes[8];
	static QTimer timer;
	static int calcFF7TextWidth(const QByteArray &ff7Text);
	static quint16 posTable[7];
	static quint8 charWidth[7][256];
	static const char *optimisedDuo[3];
	static QList<QByteArray> names;
	static int namesWidth[12];
protected:
	void paintEvent(QPaintEvent *event);
	void mousePressEvent(QMouseEvent *event);
	void mouseMoveEvent(QMouseEvent *event);
	void mouseReleaseEvent(QMouseEvent *event);
};

#endif // TEXTPREVIEW_H

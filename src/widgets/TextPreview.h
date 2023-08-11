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
#pragma once

#include <QtWidgets>
#include "core/field/Opcode.h"
#include <WindowBinFile>

class TextPreview : public QWidget
{
	Q_OBJECT
public:
	enum WindowType {
		Normal=0, WithoutFrameAndBg, Transparent, WithoutFrame
	};

	explicit TextPreview(QWidget *parent = nullptr);
	static void updateNames();
	void clear();
	void setReadOnly(bool ro);
	void setWins(const QList<FF7Window> &windows, bool update = true);
	void resetCurrentWin();
	int currentWin() const;
	FF7Window getWindow() const;
	bool setWindow(const FF7Window &win);
	qsizetype winCount() const;
	void nextWin();
	void prevWin();
	void clearWin();
	void setText(const QByteArray &textData, bool reset = true);
	int currentPage() const;
	qsizetype pageCount() const;
	void nextPage();
	void prevPage();
	void calcSize();
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
	QList<FF7Window> invisibleFf7Windows;
	QByteArray ff7Text;
	int _currentPage;
	int _currentWin;
	QList<int> pagesPos;
	int maxW, maxH;
	QPoint moveStartPosition;
	static int curFrame10;
	static bool curFrame;
	bool acceptMove;
	bool spaced_characters;
	bool readOnly;

	static int startMulticolor;
	static int multicolor;
	static WindowBinFile::FontColor fontColor;
	static QImage fontImage;
	void letter(int *x, int *y, quint8 charId, QPainter *painter, quint8 tableId = 0);
	void word(int *x, int *y, const QByteArray &charIds, QPainter *painter, quint8 tableId = 0);
	static QImage letterImage(quint8 tableId, quint8 charId);
	static void setFontColor(WindowBinFile::FontColor color, bool blink = false);
	static QList<QRgb> fontPalettes[8];
	static QTimer timer;
	static quint16 posTable[7];
	static QList<QByteArray> names;
protected:
	void paintEvent(QPaintEvent *event) override;
	void mousePressEvent(QMouseEvent *event) override;
	void mouseMoveEvent(QMouseEvent *event) override;
	void mouseReleaseEvent(QMouseEvent *event) override;
	void wheelEvent(QWheelEvent *event) override;
};

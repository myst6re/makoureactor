/****************************************************************************
 ** Makou Reactor Final Fantasy VII Field Script Editor
 ** Copyright (C) 2009-2013 Arzel Jérôme <myst6re@gmail.com>
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
#ifndef QTASKBARBUTTON_H
#define QTASKBARBUTTON_H

#include <QtGui>

#if defined(Q_OS_WIN) && defined(TASKBAR_BUTTON)
#include <shobjidl.h>
#define QTASKBAR_WIN
#endif

class QTaskBarButton : public QObject
{
    Q_OBJECT
public:
	enum State {
		Invisible, Normal, Indeterminate, Paused, Error
	};

	explicit QTaskBarButton(QWidget *mainWindow);
	virtual ~QTaskBarButton();
	void setOverlayIcon(const QPixmap &pixmap, const QString &text=QString());
	void setState(State state);
	int maximum() const;
	int minimum() const;
	State state() const;
	int value() const;
signals:
	void valueChanged(int value);
public slots:
	void reset();
	void setMaximum(int maximum);
	void setMinimum(int minimum);
	void setRange(int minimum, int maximum);
	void setValue(int value);
private:
#ifdef QTASKBAR_WIN
	WId _winId;
	ITaskbarList3 *pITask;
#endif // Q_OS_WIN
	int _minimum, _maximum, _value;
	State _state;
};
#endif // QTASKBARBUTTON_H

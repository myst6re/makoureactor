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
#include "QTaskBarButton.h"

#ifdef QTASKBAR_WIN

QTaskBarButton::QTaskBarButton(QWidget *mainWindow) :
	QObject(mainWindow), pITask(0), _minimum(0), _maximum(100),
	_value(0), _state(Invisible)
{
	_winId = mainWindow->window()->winId();

	CoInitialize(nullptr);
	HRESULT hRes = CoCreateInstance(CLSID_TaskbarList,
									nullptr, CLSCTX_INPROC_SERVER,
									IID_ITaskbarList3, (LPVOID*)&pITask);
	if (FAILED(hRes)) {
		pITask = 0;
		CoUninitialize();
		return;
	}

	pITask->HrInit();
}

QTaskBarButton::~QTaskBarButton()
{
	if (pITask) {
		pITask->Release();
		pITask = nullptr;
		CoUninitialize();
	}
}

void QTaskBarButton::setOverlayIcon(const QPixmap &pixmap, const QString &text)
{
	if (!pITask)	return;

	if (pixmap.isNull()) {
		pITask->SetOverlayIcon(_winId, nullptr, nullptr);
	} else {
		const HICON icon = pixmap.toWinHICON();
		pITask->SetOverlayIcon(_winId, icon, (wchar_t *)text.utf16());
		DestroyIcon(icon);
	}
}

void QTaskBarButton::setState(State state)
{
	if (!pITask)	return;

	TBPFLAG flag;
	switch (state) {
	case Invisible:		flag = TBPF_NOPROGRESS;
		break;
	case Indeterminate:	flag = TBPF_INDETERMINATE;
		break;
	case Paused:		flag = TBPF_PAUSED;
		break;
	case Error:			flag = TBPF_ERROR;
		break;
	default:			flag = TBPF_NORMAL;
		break;
	}

	if (S_OK == pITask->SetProgressState(_winId, flag)) {
		_state = state;
	}
}

void QTaskBarButton::setValue(int value)
{
	if (!pITask)	return;

	int completed = value - _minimum, total = _maximum - _minimum;
	if (completed < 0 || total <= 0) {
		return;
	}

	if (S_OK == pITask->SetProgressValue(_winId, completed, total)) {
		_value = value;
		emit valueChanged(value);
	}
}

#elif defined(QTASKBAR_WIN_QT5)

QTaskBarButton::QTaskBarButton(QWidget *mainWindow) :
	_taskbarButton(mainWindow), _minimum(0), _maximum(100),
    _value(0), _state(Invisible)
{
	_taskbarButton.setWindow(mainWindow->windowHandle());
	QWinTaskbarProgress *progress = _taskbarButton.progress();
	connect(progress, SIGNAL(valueChanged(int)), SIGNAL(valueChanged(int)));
}

QTaskBarButton::~QTaskBarButton()
{
}

void QTaskBarButton::setOverlayIcon(const QPixmap &pixmap, const QString &text)
{
	_taskbarButton.setOverlayIcon(pixmap);
	_taskbarButton.setOverlayAccessibleDescription(text);
}

void QTaskBarButton::setState(State state)
{
	QWinTaskbarProgress *progress = _taskbarButton.progress();

	switch (state) {
	case Invisible:
		progress->hide();
		break;
	case Indeterminate:
		progress->show();
		progress->setRange(0, 0);
		progress->resume();
		break;
	case Paused:
		progress->show();
		progress->setRange(minimum(), maximum());
		progress->pause();
		break;
	case Error:
		progress->show();
		progress->setRange(minimum(), maximum());
		progress->stop();
		break;
	default:
		progress->show();
		progress->setRange(minimum(), maximum());
		progress->resume();
		break;
	}
}

void QTaskBarButton::setValue(int value)
{
	QWinTaskbarProgress *progress = _taskbarButton.progress();
	if (progress->minimum() != minimum()) {
		progress->setMinimum(minimum());
	}
	if (progress->maximum() != maximum()) {
		progress->setMaximum(maximum());
	}
	_taskbarButton.progress()->setValue(value);
}

#else

QTaskBarButton::QTaskBarButton(QWidget *parent) :
	QObject(parent), _minimum(0), _maximum(100),
    _value(0), _state(Invisible)
{
}

QTaskBarButton::~QTaskBarButton()
{
}

void QTaskBarButton::setOverlayIcon(const QPixmap &pixmap, const QString &text)
{
	Q_UNUSED(pixmap)
	Q_UNUSED(text)
}

void QTaskBarButton::setState(State state)
{
	Q_UNUSED(state)
}

void QTaskBarButton::setValue(int value)
{
	Q_UNUSED(value)
}

#endif

int QTaskBarButton::maximum() const
{
	return _maximum;
}

int QTaskBarButton::minimum() const
{
	return _minimum;
}

QTaskBarButton::State QTaskBarButton::state() const
{
	return _state;
}

int QTaskBarButton::value() const
{
	return _value;
}

void QTaskBarButton::reset()
{
	setState(Normal);
	setValue(0);
}

void QTaskBarButton::setMaximum(int maximum)
{
	_maximum = maximum;
}

void QTaskBarButton::setMinimum(int minimum)
{
	_minimum = minimum;
}

void QTaskBarButton::setRange(int minimum, int maximum)
{
	setMinimum(minimum);
	setMaximum(maximum);
}

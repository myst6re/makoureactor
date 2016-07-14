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
#ifndef HEXLINEEDIT_H
#define HEXLINEEDIT_H

#include <QtWidgets>

class HexLineEdit : public QLineEdit
{
	Q_OBJECT
public:
	explicit HexLineEdit(QWidget *parent=0);
	explicit HexLineEdit(const QByteArray &contents, QWidget *parent=0);
	QByteArray data() const;
public slots:
	void setData(const QByteArray &contents);
signals:
	void dataChanged(const QByteArray &data);
	void dataEdited(const QByteArray &data);
private:
	QString text() const;
	void setMaxLength(int maxLength);
	void setInputMask(const QString &inputMask);

	bool _noEmit;
private slots:
	void emitDataEdited();
	void setText(const QString &);
};

#endif // HEXLINEEDIT_H

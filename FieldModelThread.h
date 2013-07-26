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
#ifndef FIELDMODELTHREAD_H
#define FIELDMODELTHREAD_H

#include <QtCore>
#include "core/field/Field.h"

class FieldModelThread : public QThread
{
	Q_OBJECT
public:
	explicit FieldModelThread(QObject *parent = 0);
	virtual ~FieldModelThread();
	void setField(Field *field);
	void setModel(int modelId, int animationId=0, bool animate=true);
	void setModels(const QList<int> &modelIds, bool animate=true);
	void cancel();
signals:
	void modelLoaded(Field *field, FieldModelFile *model, int modelId, int animationId, bool isAnimated);
protected:
	void run();
private:
	bool _canceled;
	QMutex mutex;
	static QMutex *mutexField;
	Field *_field;
	QList<int> _modelIds;
	int _animationId;
	bool _animate;
};

#endif // FIELDMODELTHREAD_H

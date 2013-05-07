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

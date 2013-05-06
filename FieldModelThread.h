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
	void setModelId(int modelId);
	void setAnimationId(int animationId);
	void setIsAnimated(bool animate);
	void cancel();
signals:
	void modelLoaded(Field *field, FieldModelFile *model, int modelId, int animationId, bool isAnimated);
protected:
	void run();
private:
	bool _canceled;
	QMutex mutex;
	static QMutex mutexField;
	Field *_field;
	int _modelId;
	int _animationId;
	bool _animate;
};

#endif // FIELDMODELTHREAD_H

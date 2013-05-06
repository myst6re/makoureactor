#include "FieldModelThread.h"

QMutex FieldModelThread::mutexField;

FieldModelThread::FieldModelThread(QObject *parent) :
	QThread(parent), _canceled(false),
	_field(0), _modelId(0), _animationId(0),
	_animate(true)
{
}

FieldModelThread::~FieldModelThread()
{
	cancel();
	wait();
}

void FieldModelThread::setField(Field *field)
{
	mutexField.lock();
	_field = field;
	mutexField.unlock();
}

void FieldModelThread::setModelId(int modelId)
{
	mutex.lock();
	_modelId = modelId;
	mutex.unlock();
}

void FieldModelThread::setAnimationId(int animationId)
{
	mutex.lock();
	_animationId = animationId;
	mutex.unlock();
}

void FieldModelThread::setIsAnimated(bool animate)
{
	mutex.lock();
	_animate = animate;
	mutex.unlock();
}

void FieldModelThread::cancel()
{
	mutex.lock();
	_canceled = true;
	mutex.unlock();
}

void FieldModelThread::run()
{
	if(!_field) {
		qWarning() << "FieldModelThread::run() -> No field provided!";
		return;
	}

	mutex.lock();
	_canceled = false;
	int modelId = _modelId;
	int animationId = _animationId;
	bool animate = _animate;
	mutexField.lock();
	Field *field = _field;
	if(_canceled) {
		mutexField.unlock();
		mutex.unlock();
		return;
	}
	FieldModelFile *fieldModel = field->fieldModel(modelId, animationId, animate);
	mutexField.unlock();
	mutex.unlock();

	if(!_canceled && fieldModel->isOpen()) {
		emit modelLoaded(field, fieldModel, modelId, animationId, animate);
	}
}

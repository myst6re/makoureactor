#ifndef IO_H
#define IO_H

#include <QtCore>

class IO
{
public:
	explicit IO(QIODevice *device);
	virtual ~IO();

	inline void setDevice(QIODevice *device) {
		_device = device;
	}

	inline QIODevice *device() const {
		return _device;
	}

	bool canRead() const;
	bool canWrite() const;
	QString errorString() const;
protected:
	inline void setError(const QString &error) {
		_error = error;
	}
private:
	QIODevice *_device;
	QString _error;
};

#endif // IO_H

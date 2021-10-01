#pragma once

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
private:
	QIODevice *_device;
};

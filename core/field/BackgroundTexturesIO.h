#ifndef BACKGROUNDTEXTURESIO_H
#define BACKGROUNDTEXTURESIO_H

#include <QtCore>
#include "BackgroundTextures.h"

class BackgroundTexturesIO
{
public:
	explicit BackgroundTexturesIO(QIODevice *device);
	virtual ~BackgroundTexturesIO();

	void setDevice(QIODevice *device);
	QIODevice *device() const;

	bool canRead() const;
	bool canWrite() const;

	virtual bool read(BackgroundTextures *textures) {
		Q_UNUSED(textures)
		return false;
	}
	virtual bool write(const BackgroundTextures *textures) {
		Q_UNUSED(textures)
		return false;
	}
private:
	QIODevice *_device;
};

class BackgroundTexturesIOPC : public BackgroundTexturesIO
{
public:
	explicit BackgroundTexturesIOPC(QIODevice *device);
	bool read(BackgroundTexturesPC *textures);
	bool write(const BackgroundTexturesPC *textures);
};

class BackgroundTexturesIOPS : public BackgroundTexturesIO
{
public:
	explicit BackgroundTexturesIOPS(QIODevice *device);
	bool read(BackgroundTexturesPS *textures);
	bool write(const BackgroundTexturesPS *textures);
};

#endif // BACKGROUNDTEXTURESIO_H

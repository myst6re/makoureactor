#ifndef BACKGROUNDIO_H
#define BACKGROUNDIO_H

#include <QtCore>
#include "BackgroundFile.h"
#include "BackgroundFilePS.h"
#include "BackgroundFilePC.h"

class BackgroundIO
{
public:
	explicit BackgroundIO(QIODevice *device);
	virtual ~BackgroundIO();

	void setDevice(QIODevice *device);
	QIODevice *device() const;

	bool canRead() const;
	bool canWrite() const;

	virtual bool read(BackgroundFile &background) const=0;
	virtual bool write(const BackgroundFile &background) const=0;
private:
	QIODevice *_device;
};

class BackgroundIOPC : public BackgroundIO
{
public:
	explicit BackgroundIOPC(QIODevice *device, QIODevice *devicePal);

	void setDevicePal(QIODevice *device);
	QIODevice *devicePal() const;

	bool canReadPal() const;
	bool canWritePal() const;

	bool read(BackgroundFile &background) const;
	bool write(const BackgroundFile &background) const;
private:
	bool openPalettes(PalettesPC &palettes) const;
	bool openTiles(BackgroundTiles &tiles) const;
	bool openTextures(BackgroundTexturesPC &textures) const;
	QIODevice *_devicePal;
};

class BackgroundIOPS : public BackgroundIO
{
public:
	explicit BackgroundIOPS(QIODevice *device, QIODevice *deviceTiles);

	void setDeviceTiles(QIODevice *device);
	QIODevice *deviceTiles() const;

	bool canReadTiles() const;
	bool canWriteTiles() const;

	bool read(BackgroundFile &background) const;
	bool write(const BackgroundFile &background) const;
private:
	bool openPalettes(PalettesPS &palettes) const;
	bool openTiles(BackgroundTiles &tiles) const;
	bool openTextures(BackgroundTexturesPS &textures) const;
	QIODevice *_deviceTiles;
};

#endif // BACKGROUNDIO_H

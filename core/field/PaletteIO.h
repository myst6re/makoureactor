#ifndef PALETTEIO_H
#define PALETTEIO_H

#include <QtCore>
#include "Palette.h"

class PaletteIO
{
public:
	PaletteIO();
	explicit PaletteIO(QIODevice *device);
	virtual ~PaletteIO();

	void setDevice(QIODevice *device);
	QIODevice *device() const;

	bool canRead() const;
	bool canWrite() const;

	bool read(Palettes &palettes) const;
	bool write(const Palettes &palettes) const;
protected:
	virtual Palette *createPalette(const char *data) const=0;
	virtual bool readAfter(Palettes &palettes) const {
		Q_UNUSED(palettes)
		return true;
	}
	virtual bool writeAfter(const Palettes &palettes) const {
		Q_UNUSED(palettes)
		return true;
	}
private:
	QIODevice *_device;
};

class PaletteIOPC : public PaletteIO
{
public:
	explicit PaletteIOPC(QIODevice *device, QIODevice *deviceAlpha);

	void setDeviceAlpha(QIODevice *device);
	QIODevice *deviceAlpha() const;

	bool canReadAlpha() const;
	bool canWriteAlpha() const;
protected:
	inline Palette *createPalette(const char *data) const {
		return new PalettePC(data);
	}
	bool readAfter(Palettes &palettes) const;
	bool writeAfter(const Palettes &palettes) const;
private:
	QIODevice *_deviceAlpha;
};

class PaletteIOPS : public PaletteIO
{
public:
	explicit PaletteIOPS(QIODevice *device);
protected:
	inline Palette *createPalette(const char *data) const {
		return new PalettePS(data);
	}
};

#endif // PALETTEIO_H

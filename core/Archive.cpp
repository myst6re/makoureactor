#include "Archive.h"

/*!
	\class Archive

	\brief The Archive class is a device list in a file system
	or an archive file.

	\sa QIODevice
*/

/*!
 * Constructs a new empty archive.
 */
Archive::Archive() :
	_archiveIO(new QFile())
{
}

/*!
 * Constructs a new archive object to represent the archive with the given \a filename.
 */
Archive::Archive(const QString &filename) :
	_archiveIO(new QFile(filename))
{
}

/*!
 * Constructs a new archive object to represent the archive with the given \a device.
 */
Archive::Archive(QFile *device) :
	_archiveIO(device)
{
}

/*!
 * Destroys the archive object, closing it if necessary.
 */
Archive::~Archive()
{
	_archiveIO->deleteLater();
}

/*!
 * Returns the data for the file named \a filePath.
 * \sa file(), modifiedFile(), modifiedFileData()
 */
QByteArray Archive::fileData(const QString &filePath)
{
	QIODevice *io = file(filePath);
	if(io == NULL || !io->open(QIODevice::ReadOnly)) {
		qWarning() << "Archive::fileData error";
		return QByteArray();
	}
	QByteArray data = io->readAll();
	io->close();
	return data;
}

/*!
 * Returns the data, modified by setData if modified, for the file named \a filePath.
 * \sa file(), fileData(), modifiedFile()
 */
QByteArray Archive::modifiedFileData(const QString &filePath)
{
	QIODevice *io = modifiedFile(filePath);
	if(io == NULL || !io->open(QIODevice::ReadOnly)) {
		qWarning() << "Archive::modifiedFileData error";
		return QByteArray();
	}
	QByteArray data = io->readAll();
	io->close();
	return data;
}

/*!
 * \overload
 *
 * Change the \a data for the file named \a filePath.
 * Returns false if the file doesn't exists; otherwise
 * returns true.
 * \sa setFile()
 */
bool Archive::setFileData(const QString &filePath, const QByteArray &data)
{
	QBuffer *buf = new QBuffer();
	buf->setData(data);
	return setFile(filePath, buf);
}

/*!
 * \overload
 *
 * Add a new file named \a filePath with \a data.
 * Returns false if the file exists; otherwise returns
 * true.
 * \sa addFile()
 */
bool Archive::addFileData(const QString &filePath, const QByteArray &data)
{
	QBuffer *buf = new QBuffer();
	buf->setData(data);
	return addFile(filePath, buf);
}

/*!
 * Opens the archive, returning true if successful;
 * otherwise false.
 * \sa isOpen(), close()
 */
bool Archive::open()
{
	return !_archiveIO->exists() // Create the file
			|| (_archiveIO->open(QIODevice::ReadOnly)
				&& openHeader());
}

/*!
 * Returns true if the archive is open;
 * returns false otherwise.
 * \sa open(), close()
 */
bool Archive::isOpen() const
{
	return _archiveIO->isOpen();
}

/*!
 * Closes the file.
 * \sa open(), isOpen()
 */
void Archive::close()
{
	_archiveIO->close();
}

/*!
 * Returns the name set by setFileName() or to the
 * constructors.
 * \sa setFileName(), QFile::fileName()
 */
QString Archive::fileName() const
{
	return _archiveIO->fileName();
}

/*!
 * Sets the \a name of the file.
 * Do not call this function if the file has already been opened.
 * \sa fileName(), QFile::setFileName()
 */
void Archive::setFileName(const QString &fileName)
{
	_archiveIO->setFileName(fileName);
}

/*!
 * Returns the last error message.
 * \sa unsetError(), error()
 */
QString Archive::errorString() const
{
	return _errorString.isEmpty()
			? QLatin1String(QT_TRANSLATE_NOOP(Archive, ("Unknown error")))
			: _errorString;
}

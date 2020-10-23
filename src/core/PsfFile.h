#ifndef PSFFILE_H
#define PSFFILE_H

#include <QtCore>

class PsfTags
{
public:
	PsfTags();
	explicit PsfTags(const QString &lib);
	bool open(const QString &config);
	QString save() const;
	inline PsfTags &setLib(const QString &lib) {
		return setCustom("_lib", lib);
	}
	inline PsfTags &setAuthor(const QString &author) {
		return setCustom("psfby", author);
	}
	PsfTags &setLengthS(quint32 seconds);
	inline PsfTags &setFade(quint32 fade) {
		return setCustom("fade", fade);
	}
	inline PsfTags &setVolume(quint32 volume) {
		return setCustom("volume", volume);
	}
	inline PsfTags &setTitle(const QString &title) {
		return setCustom("title", title);
	}
	inline PsfTags &setArtist(const QString &artist) {
		return setCustom("artist", artist);
	}
	inline PsfTags &setGame(const QString &game) {
		return setCustom("game", game);
	}
	inline PsfTags &setGenre(const QString &genre) {
		return setCustom("genre", genre);
	}
	inline PsfTags &setComment(const QString &comment) {
		return setCustom("comment", comment);
	}
	inline PsfTags &setYear(quint32 year) {
		return setCustom("year", year);
	}
	inline PsfTags &setCopyright(const QString &copyright) {
		return setCustom("copyright", copyright);
	}
	inline PsfTags &setReplayGainTrack(float gain_db, float peak) {
		return setReplayGain("track", gain_db, peak);
	}
	inline PsfTags &setReplayGainAlbum(float gain_db, float peak) {
		return setReplayGain("album", gain_db, peak);
	}
	inline PsfTags &setCustom(const QString &name, const QString &value) {
		_tags.insert(name, value);
		return *this;
	}
	inline PsfTags &setCustom(const QString &name, int value) {
		return setCustom(name, QString::number(value));
	}
private:
	PsfTags &setReplayGain(const QString &type, float gain_db, float peak);
	QMap<QString, QString> _tags;
};

class PsfFile
{
public:
	PsfFile();
	PsfFile(const QByteArray &data, const PsfTags &tags);
	static PsfFile fromAkao(const QByteArray &akaoData, const PsfTags &tags);
	bool open(const QByteArray &data);
	QByteArray save() const;
	inline const QByteArray &data() const {
		return _data;
	}
	inline QByteArray &data() {
		return _data;
	}
	inline const QByteArray &special() const {
		return _special;
	}
	inline const PsfTags &tags() const {
		return _tags;
	}
	inline PsfTags &tags() {
		return _tags;
	}
	QByteArray akao() const;
private:
	QByteArray _special, _data;
	PsfTags _tags;
};

#endif // PSFFILE_H

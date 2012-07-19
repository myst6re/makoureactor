#include "NeoMidgar.h"

bool NeoMidgar::count(const QString &path)
{
	QFile file(path);
	if(!file.open(QIODevice::WriteOnly))	return false;

	for(int fieldID=0 ; fieldID<archive->size() ; ++fieldID) {
		Field *field = archive->field(fieldID);
		if(field != NULL) {
			file.write(QString("%1     %2\r\n").arg(field->getName()).arg(field->listUsedTexts().size()).toLatin1());
		}
	}

	return true;
}

bool NeoMidgar::neop(const QString &source, const QString &cible)
{
	bool jp = Config::value("jp_txt", false).toBool();
	QFile srcFile(source);
	if(!srcFile.open(QIODevice::ReadOnly | QIODevice::Text))	return false;
	QFile cibleFile(cible);
	if(!cibleFile.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text))	return false;

	QRegExp regExp("^(\\(\\d+, (\\d+), (\\d+), '.*', '.*', ').*(', \\d+\\)[,;]\n?)$");

	while(srcFile.bytesAvailable()) {
		QByteArray line = srcFile.readLine();
		if(line.startsWith("INSERT INTO `nm_textes` (`id`, `file_id`, `reference`, `proposal_fre`, `proposal_eng`, `proposal_jap`, `finished`) VALUES")) {
			cibleFile.write(line);
		} else if(regExp.exactMatch(line)) {
			QStringList capturedTexts = regExp.capturedTexts();
			int file_id = capturedTexts.at(2).toInt();
			int textID = capturedTexts.at(3).toInt()-1;
			if(map.contains(file_id)) {
				QString name = map[file_id];
				Field *field = archive->field(name);
				if(field != NULL) {
					if(textID < field->getNbTexts()) {
						QString text = field->getText(textID)->getText(jp).replace("'", "''").replace("\n", "\\n").replace("\t", "\\t");
						cibleFile.write(QString::fromUtf8(line).replace(regExp, QString("\\1%1\\4").arg(text)).toUtf8());
					} else {
						qWarning() << "Bad TextID" << name << textID << field->getNbTexts();
						cibleFile.write(line);
					}
				} else {
					qWarning() << "field introuvable" << name;
					cibleFile.write(line);
				}
			} else {
				qWarning() << "file_id invalide" << file_id;
				cibleFile.write(line);
			}
		} else {
			qWarning() << "Ligne inconnue" << line;
			return false;
//			cibleFile.write(line);
		}
	}

	return true;
}

bool NeoMidgar::save(const QString &path)
{
//	bool jp = Config::value("jp_txt", false).toBool();
//	int f=1, l=0;
//	QFile file(QString("%1-%2").arg(f++).arg(path));
//	if(!file.open(QIODevice::WriteOnly))	return false;

//	bool coma=true;

//	file.write(QString("CREATE TEMP TABLE t(f int(2) unsigned NOT NULL, r int(2) unsigned NOT NULL, p varchar(9999) CHARACTER SET utf8 COLLATE utf8_swedish_ci NOT NULL);\n").toUtf8());
//	file.write(QString("INSERT INTO t VALUES\n").toUtf8());
//	for(int fieldID=0 ; fieldID<archive->size() ; ++fieldID) {
//		Field *field = archive->field(fieldID);
//		if(field != NULL) {
//			if(map.contains(field->getName().toLower())) {
////				qWarning() << field->getName().toLower();
//				int id = map.value(field->getName().toLower());

//				for(int textID=0 ; textID<field->getNbTexts() ; ++textID) {
//					QString text = field->getText(textID)->getText(jp).replace("'", "''").replace("\n", "\\n").replace("\t", "\\t");
////					file.write(QString("UPDATE nm_textes SET proposal_jap='%1' WHERE file_id=%2 AND reference=%3;\n")
////							   .arg(text).arg(id).arg(textID+1).toUtf8());
//					file.write(QString("(%2, %3, '%1')")
//							   .arg(text).arg(id).arg(textID+1).toUtf8());

//					++l;

//					if(l % 300 == 0) {
////						file.close();
////						file.setFileName(QString("%1-%2").arg(f++).arg(path));
////						if(!file.open(QIODevice::WriteOnly))	return false;
//						file.write(QString(";\nINSERT INTO t VALUES\n").toUtf8());
//						coma = false;
//					} else {
//						file.write(QString(",\n").toUtf8());
//						coma = true;
//					}
//				}
//			} else {
//				qWarning() << "écran inconnu" << field->getName().toLower();
//			}
//		}
//	}
//	file.write(QString("UPDATE nm_textes SET proposal_jap=(SELECT p FROM t WHERE f=file_id AND r=reference) WHERE file_id IN (SELECT f FROM t) AND reference IN (SELECT r FROM t);").toUtf8());

//	file.close();

	return true;
}

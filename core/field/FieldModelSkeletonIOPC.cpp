#include "FieldModelSkeletonIOPC.h"
#include "FieldModelPartIOPC.h"

FieldModelSkeletonIOPC::FieldModelSkeletonIOPC(QIODevice *io) :
	FieldModelSkeletonIO(io)
{
}

bool FieldModelSkeletonIOPC::read(FieldModelSkeleton &skeleton, QMultiMap<int, QStringList> &rsdFiles) const
{
	if (!canRead()) {
		return false;
	}

	bool ok;
	QString line;
	quint32 boneCount=0;

	do {
		line = QString(device()->readLine()).trimmed();
		if (line.startsWith(QString(":BONES "))) {
			boneCount = line.mid(7).toUInt(&ok);
			if (!ok) {
				return false;
			}
			if (boneCount == 0) {
				boneCount = 1; // Null HRC fix
			}
			break;
		}
	} while (device()->canReadLine());

	if (boneCount == 0) {
		return false;
	}

	int nbP, lineType = 0;
	quint32 boneID = 0;
	FieldModelBone bone;
	bone.parent = 0;
	QMap<QString, int> nameToId;
	QStringList rsdlist;
	nameToId.insert("root", -1);

	while (device()->canReadLine() && boneID < boneCount) {
		line = QString(device()->readLine()).trimmed();
		if (line.isEmpty() || line.startsWith(QChar('#'))) {
			continue;
		}

		switch (lineType) {
		case 0: // Current
			nameToId.insert(line, boneID);
			break;
		case 1: // Parent
			bone.parent = nameToId.value(line, -1);
			break;
		case 2: // Length
			bone.size = -line.toFloat(&ok) / MODEL_SCALE_PC;
			if (!ok) {
				return false;
			}
			skeleton.addBone(bone);
			break;
		case 3: // RSD list
			rsdlist = line.split(' ', QString::SkipEmptyParts);
			if (rsdlist.size() < 1) {
				return false;
			}
			nbP = rsdlist.first().toUInt(&ok);
			if (ok && nbP != 0) {
				rsdlist.removeFirst();
				if (rsdlist.size() == nbP) {
					rsdFiles.insert(boneID, rsdlist);
				}
			}
			++boneID;
			break;
		}
		lineType = (lineType + 1) % 4;
	}

	return boneID == boneCount;
}

bool FieldModelSkeletonIOPC::write(const FieldModelSkeleton &part) const
{
	Q_UNUSED(part)
	return false; // TODO
}

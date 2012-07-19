/****************************************************************************
 ** Makou Reactor Final Fantasy VII Field Script Editor
 ** Copyright (C) 2009-2012 Arzel Jérôme <myst6re@gmail.com>
 **
 ** This program is free software: you can redistribute it and/or modify
 ** it under the terms of the GNU General Public License as published by
 ** the Free Software Foundation, either version 3 of the License, or
 ** (at your option) any later version.
 **
 ** This program is distributed in the hope that it will be useful,
 ** but WITHOUT ANY WARRANTY; without even the implied warranty of
 ** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 ** GNU General Public License for more details.
 **
 ** You should have received a copy of the GNU General Public License
 ** along with this program.  If not, see <http://www.gnu.org/licenses/>.
 ****************************************************************************/
#ifndef FIELDMODELLOADER_H
#define FIELDMODELLOADER_H

#include <QtGui>

class FieldModelLoader
{
public:
    FieldModelLoader();
	bool isLoaded() const;
	bool isModified() const;
	void setModified(bool modified);
	void load(const QByteArray &contenu, const QString &name);
	QByteArray save(const QByteArray &contenu, const QString &name) const;

	quint16 typeHRC;
	QStringList model_nameChar;
	QStringList model_nameHRC;
	QList<QStringList> model_anims;
	QList<quint16> model_unknown;
	QList<quint16> model_typeHRC;
	QList< QList<QRgb> > colors;

private:
	bool loaded, modified;
};

#endif // FIELDMODELLOADER_H

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
#include <QApplication>
#include "Window.h"
#include "core/Var.h"
#include "core/Config.h"
#include "Data.h"

int main(int argc, char *argv[])
{
	QGLFormat::setDefaultFormat(QGLFormat(QGL::DirectRendering));

	QApplication app(argc, argv);
	app.setWindowIcon(QIcon(":/images/logo-shinra.png"));
#if (QT_VERSION < QT_VERSION_CHECK(5, 0, 0))
	QTextCodec::setCodecForTr(QTextCodec::codecForName("UTF-8"));
#endif

	Config::set();

	QString lang = QLocale::system().name().toLower();
	lang = Config::value("lang", lang.left(lang.indexOf("_"))).toString();
	QTranslator translator1;
	if(translator1.load("qt_" % lang, app.applicationDirPath()) || translator1.load("qt_" % lang, QLibraryInfo::location(QLibraryInfo::TranslationsPath)))
		app.installTranslator(&translator1);
	QTranslator translator2;
	if(translator2.load("Makou_Reactor_" % lang, Config::programResourceDir())) {
		app.installTranslator(&translator2);
		Config::setValue("lang", lang);
	} else {
		Config::setValue("lang", "fr");
	}

	if(!Var::load()) {
		QMessageBox::warning(0, QObject::tr("Erreur"), QObject::tr("Le fichier 'var.cfg' n'a pas pu être chargé.\nVérifiez que ce fichier est valide ou supprimez-le."));
	}
	if(!Data::load()) {
		qWarning() << "Error loading data!";
	}
	
	Window window;
	window.show();
	if(argc > 1) {
		window.openFile(argv[1]);
	}

	return app.exec();
}

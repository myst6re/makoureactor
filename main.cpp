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
#include "core/FF7Font.h"
#include "Parameters.h"
#include "CLI.h"

int main(int argc, char *argv[])
{
#ifdef MR_CONSOLE
	QCoreApplication app(argc, argv);
	QCoreApplication::setApplicationName(PROG_NAME);
	QCoreApplication::setApplicationVersion(PROG_VERSION);
#ifdef Q_OS_WIN
	QTextCodec::setCodecForLocale(QTextCodec::codecForName("IBM 850"));
#endif
	Config::set();
	CLI::exec();

	QTimer::singleShot(0, &app, SLOT(quit()));
#else

	QApplication app(argc, argv);
	app.setWindowIcon(QIcon(":/images/logo-shinra.png"));

	Config::set();

	QString lang = QLocale::system().name().toLower();
	lang = Config::value("lang", lang.left(lang.indexOf("_"))).toString();
	QTranslator translator1;
	if (translator1.load("qt_" % lang, app.applicationDirPath()) || translator1.load("qt_" % lang, QLibraryInfo::location(QLibraryInfo::TranslationsPath)))
		app.installTranslator(&translator1);
	QTranslator translator2;
	if (translator2.load("Makou_Reactor_" % lang, Config::programResourceDir())) {
		app.installTranslator(&translator2);
		Config::setValue("lang", lang);
	} else {
		Config::setValue("lang", "en");
	}

	if (Config::value("dark_theme", false).toBool()) {
		qApp->setStyle(QStyleFactory::create("Fusion"));
		QPalette darkPalette;
		QColor disabledColor = QColor(127, 127, 127);
		darkPalette.setColor(QPalette::Window, QColor(0x19, 0x19, 0x19));
		darkPalette.setColor(QPalette::WindowText, Qt::white);
		darkPalette.setColor(QPalette::Base, QColor(0x20, 0x20, 0x20));
		darkPalette.setColor(QPalette::AlternateBase, QColor(0x27, 0x27, 0x27));
		darkPalette.setColor(QPalette::ToolTipBase, QColor(0xFE, 0xFE, 0xFE));
		darkPalette.setColor(QPalette::ToolTipText, QColor(0xFE, 0xFE, 0xFE));
		darkPalette.setColor(QPalette::Text, QColor(0xFE, 0xFE, 0xFE));
		darkPalette.setColor(QPalette::Disabled, QPalette::Text, disabledColor);
		darkPalette.setColor(QPalette::Button, QColor(0x20, 0x20, 0x20));
		darkPalette.setColor(QPalette::ButtonText, Qt::white);
		darkPalette.setColor(QPalette::Disabled, QPalette::ButtonText, disabledColor);
		darkPalette.setColor(QPalette::BrightText, Qt::red);
		darkPalette.setColor(QPalette::Link, QColor(42, 130, 218));
		darkPalette.setColor(QPalette::LinkVisited, QColor(42, 130, 218));

		darkPalette.setColor(QPalette::Highlight, QColor(0x77, 0x77, 0x77));
		darkPalette.setColor(QPalette::HighlightedText, Qt::white);
		darkPalette.setColor(QPalette::Disabled, QPalette::HighlightedText, disabledColor);

		darkPalette.setColor(QPalette::Light, QColor(0x34, 0x34, 0x34));
		darkPalette.setColor(QPalette::Midlight, QColor(0x27, 0x27, 0x27));
		darkPalette.setColor(QPalette::Dark, QColor(0x7, 0x7, 0x7));
		darkPalette.setColor(QPalette::Mid, QColor(0x14, 0x14, 0x14));
		darkPalette.setColor(QPalette::Shadow, Qt::black);

#if (QT_VERSION > QT_VERSION_CHECK(5, 11, 0))
		darkPalette.setColor(QPalette::PlaceholderText, QColor(0x53, 0x53, 0x53));
#endif

		qApp->setPalette(darkPalette);

		qApp->setStyleSheet("QToolTip { color: #ffffff; background-color: #2a82da; border: 1px solid white; }");
	}

	if (!Var::load()) {
		QMessageBox::warning(0, QObject::QCoreApplication::translate("main", "Error"), QObject::QCoreApplication::translate("main", "The file 'var.cfg' could not be loaded.\nMake sure it is valid or delete it."));
	}

	if (!Data::load()) {
		qWarning() << "Error loading data!";
	}

	/* if (!FF7Font::listFonts()) {
		QMessageBox::critical(nullptr, QObject::QCoreApplication::translate("main", "Data loading"), QObject::QCoreApplication::translate("main", "Fonts couldn't be loaded!"));
		return -1;
	} */

	Window *window = new Window;
	window->show();
	if (argc > 1) {
		window->openFile(argv[1]);
	}
#endif

	return app.exec();
}

#include "FontManager.h"
#include "Data.h"

FontManager::FontManager(QWidget *parent) :
	QDialog(parent, Qt::Dialog | Qt::WindowCloseButtonHint)
{
	setWindowTitle(tr("Gestionnaire de polices de caractères"));

	fontWidget = new FontWidget(this);
	fontWidget->setWindowBinFile(&Data::windowBin);


	QVBoxLayout *layout = new QVBoxLayout(this);
	layout->addWidget(fontWidget);
}

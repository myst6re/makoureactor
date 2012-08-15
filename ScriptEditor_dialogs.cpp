#include "ScriptEditor_dialogs.h"

GotoPage::GotoPage(QWidget *parent) :
	QWidget(parent)
{
	saut = new QSpinBox();
	
	sens = new QComboBox();
	sens->addItem(tr("En avant"));
	sens->addItem(tr("En arrière"));
	
	range = new QComboBox();
	range->addItem(QString("0 -> 255"));
	range->addItem(QString("-32768 -> 32767"));
	setRange(0);
	QObject::connect(range, SIGNAL(currentIndexChanged(int)), this, SLOT(setRange(int)));

	QGridLayout *layout = new QGridLayout();
	layout->addWidget(new QLabel(tr("Nombre d'octets à sauter")), 0, 0);
	layout->addWidget(saut, 0, 1);
	layout->addWidget(new QLabel(tr("Sens")), 1, 0);
	layout->addWidget(sens, 1, 1);
	layout->addWidget(new QLabel(tr("Valeur maximale du saut")), 2, 0);
	layout->addWidget(range, 2, 1);
	layout->setRowStretch(3, 1);
	
	setLayout(layout);
}

void GotoPage::setRange(int rangeIndex)
{
	saut->setRange(rangeIndex ? -32768 : 0, rangeIndex ? 32767 : 255);
}

void GotoPage::fill(int opcode, QByteArray params)
{
	qint16 valeur_saut = 0;
	if(opcode==0x10 || opcode==0x12)
	{
		range->setCurrentIndex(0);
		valeur_saut = (quint8)params.at(0);
	}
	else
	{
		range->setCurrentIndex(1);
		memcpy(&valeur_saut, params.mid(0,2), 2);
	}
	saut->setValue(valeur_saut+1);
	sens->setCurrentIndex(opcode==0x12 || opcode==0x13);
}

QByteArray GotoPage::save()
{
	QByteArray data;
	qint16 valeur_saut = saut->value()-1;
	quint8 opcode;
	
	//OPCODE
	if(sens->currentIndex())// En arrière
		opcode = range->currentIndex() ? 0x13 : 0x12;
	else// En avant
		opcode = range->currentIndex() ? 0x11 : 0x10;
	
	data.append((char)opcode);
	//PARAMS
	data.append((char *)&valeur_saut, opcode==0x10 || opcode==0x12 ? 1 : 2);
	
	return data;
}

ReturnPage::ReturnPage(QWidget *parent) : QWidget(parent)
{
	callback = new QGroupBox(tr("Exécuter un script après le return"));
	callback->setCheckable(true);
	
	script = new QSpinBox();
	script->setRange(0, 31);
	
	priority = new QSpinBox();
	priority->setRange(0, 7);

	QGridLayout *layout2 = new QGridLayout(callback);
	layout2->addWidget(new QLabel(tr("Script à exécuter")), 0, 0);
	layout2->addWidget(script, 0, 1);
	layout2->addWidget(new QLabel(tr("Priorité")), 1, 0);
	layout2->addWidget(priority, 1, 1);
	
	QVBoxLayout *layout1 = new QVBoxLayout();
	layout1->addWidget(callback);
	layout1->addStretch();
	
	setLayout(layout1);
}

void ReturnPage::fill(int opcode, QByteArray params)
{
	if(opcode==0x00)
		callback->setChecked(false);
	else
	{
		callback->setChecked(true);
		quint8 valeur = (quint8)params.at(0);
		script->setValue(valeur & 0x1F);
		priority->setValue((valeur >> 5) & 7);
	}
}

QByteArray ReturnPage::save()
{
	QByteArray data;
	
	if(callback->isChecked())
	{
		quint8 valeur = script->value();
		valeur |= priority->value() << 5;
		data.append((char)0x07);
		data.append((char)valeur);
	}
	else
		data.append((char)0x00);
	
	return data;
}

ExecPage::ExecPage(QWidget *parent) : QWidget(parent)
{
	group = new QComboBox();
	group->addItems(DATA::currentGrpScriptNames);

	script = new QSpinBox();
	script->setRange(0, 31);
	
	priority = new QSpinBox();
	priority->setRange(0, 7);
	
	type = new QComboBox();
	type->addItem("-");
	type->addItem(tr("Ne pas exécuter si le script est déjà en cours d'exécution"));
	type->addItem(tr("Attendre la fin de l'exécution pour continuer"));

	QGridLayout *layout = new QGridLayout();
	layout->addWidget(new QLabel(tr("Groupe")), 0, 0);
	layout->addWidget(group, 0, 1);
	layout->addWidget(new QLabel(tr("Script à exécuter")), 1, 0);
	layout->addWidget(script, 1, 1);
	layout->addWidget(new QLabel(tr("Priorité")), 2, 0);
	layout->addWidget(priority, 2, 1);
	layout->addWidget(new QLabel(tr("Extra")), 3, 0);
	layout->addWidget(type, 3, 1);
	layout->setRowStretch(4, 1);
	
	setLayout(layout);
}

void ExecPage::fill(int opcode, QByteArray params)
{
	if(opcode==0x02)
		type->setCurrentIndex(0);
	else if(opcode==0x01)
		type->setCurrentIndex(1);
	else
		type->setCurrentIndex(2);
	
	group->setCurrentIndex((quint8)params.at(0));
	quint8 valeur = (quint8)params.at(1);
	script->setValue(valeur & 0x1F);
	priority->setValue((valeur >> 5) & 7);
}

QByteArray ExecPage::save()
{
	QByteArray data;
	
	if(type->currentIndex()==0)
		data.append((char)0x02);
	else if(type->currentIndex()==1)
		data.append((char)0x01);
	else
		data.append((char)0x03);

	data.append((char)group->currentIndex());
	quint8 valeur = script->value();
	valeur |= priority->value() << 5;
	data.append((char)valeur);
	
	return data;
}

ExecCharPage::ExecCharPage(QWidget *parent) : QWidget(parent)
{
	character = new QSpinBox();
	character->setRange(0, 255);

	script = new QSpinBox();
	script->setRange(0, 31);
	
	priority = new QSpinBox();
	priority->setRange(0, 7);

	type = new QComboBox();
	type->addItem("-");
	type->addItem(tr("Ne pas exécuter si le script est déjà en cours d'exécution"));
	type->addItem(tr("Attendre la fin de l'exécution pour continuer"));

	QGridLayout *layout = new QGridLayout();
	layout->addWidget(new QLabel(tr("Personnage")), 0, 0);
	layout->addWidget(character, 0, 1);
	layout->addWidget(new QLabel(tr("Script à exécuter")), 1, 0);
	layout->addWidget(script, 1, 1);
	layout->addWidget(new QLabel(tr("Priorité")), 2, 0);
	layout->addWidget(priority, 2, 1);
	layout->addWidget(new QLabel(tr("Extra")), 3, 0);
	layout->addWidget(type, 3, 1);
	layout->setRowStretch(4, 1);
	
	setLayout(layout);
}

void ExecCharPage::fill(int opcode, QByteArray params)
{
	if(opcode==0x05)
		type->setCurrentIndex(0);
	else if(opcode==0x04)
		type->setCurrentIndex(1);
	else
		type->setCurrentIndex(2);
	
	character->setValue((quint8)params.at(0));
	quint8 valeur = (quint8)params.at(1);
	script->setValue(valeur & 0x1F);
	priority->setValue((valeur >> 5) & 7);
}

QByteArray ExecCharPage::save()
{
	QByteArray data;
	
	if(type->currentIndex()==0)
		data.append((char)0x05);
	else if(type->currentIndex()==1)
		data.append((char)0x04);
	else
		data.append((char)0x06);

	data.append((char)character->value());
	quint8 valeur = script->value();
	valeur |= priority->value() << 5;
	data.append((char)valeur);
	
	return data;
}

IfPage::IfPage(QWidget *parent) : QWidget(parent)
{
	bank1 = new QSpinBox();
	bank1->setRange(1, 15);
	bank1->hide();
	valeur1 = new QSpinBox();
	
	QHBoxLayout *hlayout1 = new QHBoxLayout();
	hlayout1->addWidget(bank1);
	hlayout1->addWidget(valeur1);
	
	operateur = new QComboBox();
	operateur->addItems(DATA::operateur_names);
	
	bank2 = new QSpinBox();
	bank2->setRange(1, 15);
	bank2->hide();
	valeur2 = new QSpinBox();
	
	QHBoxLayout *hlayout2 = new QHBoxLayout();
	hlayout2->addWidget(bank2);
	hlayout2->addWidget(valeur2);
	
	QStringList strlist;
	strlist << tr("Valeur") << tr("Variable");
	
	type_valeur1 = new QComboBox();
	type_valeur1->addItems(strlist);
	QObject::connect(type_valeur1, SIGNAL(currentIndexChanged(int)), this, SLOT(setType1(int)));
	
	type_valeur2 = new QComboBox();
	type_valeur2->addItems(strlist);
	QObject::connect(type_valeur2, SIGNAL(currentIndexChanged(int)), this, SLOT(setType2(int)));
	
	range_valeurs = new QComboBox();
	range_valeurs->addItem(QString("0 -> 255"));
	range_valeurs->addItem(QString("0 -> 65535"));
	range_valeurs->addItem(QString("-32768 -> 32767"));
	QObject::connect(range_valeurs, SIGNAL(currentIndexChanged(int)), this, SLOT(setRange_valeurs(int)));

	saut = new QSpinBox();
	
	range_saut = new QComboBox();
	range_saut->addItem(tr("255 octets"));
	range_saut->addItem(tr("65535 octets"));
	QObject::connect(range_saut, SIGNAL(currentIndexChanged(int)), this, SLOT(setRange_saut(int)));
	
	QGridLayout *layout = new QGridLayout();
	layout->addWidget(new QLabel(tr("Test à effectuer")), 0, 0);
	layout->addLayout(hlayout1, 0, 1);
	layout->addWidget(operateur, 0, 2);
	layout->addLayout(hlayout2, 0, 3);
	layout->addWidget(type_valeur1, 1, 1);
	layout->addWidget(type_valeur2, 1, 3);
	
	layout->addWidget(new QLabel(tr("Étendue des valeurs à tester")), 2, 0);
	layout->addWidget(range_valeurs, 2, 1, 1, 3);
	layout->addWidget(new QLabel(tr("Nombre d'octets à sauter")), 3, 0);
	layout->addWidget(saut, 3, 1, 1, 3);
	layout->addWidget(new QLabel(tr("Valeur maximale du saut")), 4, 0);
	layout->addWidget(range_saut, 4, 1, 1, 3);
	layout->setRowStretch(5, 1);
	
	setLayout(layout);
}

void IfPage::setType1(int index)
{
	if(index==1) {
		bank1->show();
		valeur1->setRange(0, 255);
	} else {
		bank1->hide();
		setRange_valeurs(range_valeurs->currentIndex());
	}
}

void IfPage::setType2(int index)
{
	if(index==1) {
		bank2->show();
		valeur2->setRange(0, 255);
	} else {
		bank2->hide();
		setRange_valeurs(range_valeurs->currentIndex());
	}
}

void IfPage::setRange_valeurs(int rangeIndex)
{
	if(rangeIndex==0) {
		valeur1->setRange(0, 255);
		valeur2->setRange(0, 255);
	} else if(rangeIndex==1) {
		valeur1->setRange(0, 65535);
		valeur2->setRange(0, 65535);
	} else {
		valeur1->setRange(-32768, 32767);
		valeur2->setRange(-32768, 32767);
	}
}

void IfPage::setRange_saut(int rangeIndex)
{
	saut->setRange(0, rangeIndex ? 65535 : 255);
}

void IfPage::fill(int opcode, QByteArray params)
{
	quint8 b1 = B1(params.at(0)), b2 = B2(params.at(0)), origin;
	quint16 valeur;
	qint16 valeur_s;
	
	bank1->setValue(b1);
	type_valeur1->setCurrentIndex(b1!=0);
	
	bank2->setValue(b2);
	type_valeur2->setCurrentIndex(b2!=0);
	
	if(opcode==0x14 || opcode==0x15) {
		range_valeurs->setCurrentIndex(0);
		valeur1->setValue((quint8)params.at(1));
		valeur2->setValue((quint8)params.at(2));
		origin = 3;
	}
	else if(opcode==0x16 || opcode==0x17) {
		range_valeurs->setCurrentIndex(2);
		memcpy(&valeur_s, params.mid(1,2), 2);
		valeur1->setValue(valeur_s);
		memcpy(&valeur_s, params.mid(3,2), 2);
		valeur2->setValue(valeur_s);
		origin = 5;
	}
	else {
		range_valeurs->setCurrentIndex(1);
		memcpy(&valeur, params.mid(1,2), 2);
		valeur1->setValue(valeur);
		memcpy(&valeur, params.mid(3,2), 2);
		valeur2->setValue(valeur);
		origin = 5;
	}
	
	operateur->setCurrentIndex(params.at(origin));
	
	if(opcode==0x14 || opcode==0x16 || opcode==0x18) {
		range_saut->setCurrentIndex(0);
		saut->setValue(params.at(origin+1));
	}
	else {
		range_saut->setCurrentIndex(1);
		memcpy(&valeur, params.mid(origin+1,2), 2);
		saut->setValue(valeur);
	}
}

QByteArray IfPage::save()
{
	QByteArray data;
	quint16 valeur;
	qint16 valeur_s;
	quint8 opcode, bank;
	
	if(range_valeurs->currentIndex()==0)
		opcode = range_saut->currentIndex()==0 ? 0x14 : 0x15;
	else if(range_valeurs->currentIndex()==2)
		opcode = range_saut->currentIndex()==0 ? 0x16 : 0x17;
	else
		opcode = range_saut->currentIndex()==0 ? 0x18 : 0x19;
	
	data.append((char)opcode);
	bank = bank1->isVisible() ? (bank1->value() & 0xF) << 4 : 0;
	bank |= bank2->isVisible() ? bank2->value() & 0xF : 0;
	data.append((char)bank);
	
	if(opcode==0x14 || opcode==0x15) {
		data.append((char)valeur1->value());
		data.append((char)valeur2->value());
	} else if(opcode==0x16 || opcode==0x17) {
		valeur_s = valeur1->value();
		data.append((char *)&valeur_s, 2);
		valeur_s = valeur2->value();
		data.append((char *)&valeur_s, 2);
	} else {
		valeur = valeur1->value();
		data.append((char *)&valeur, 2);
		valeur = valeur2->value();
		data.append((char *)&valeur, 2);
	}
	
	data.append((char)operateur->currentIndex());
	
	if(opcode==0x14 || opcode==0x16 || opcode==0x18) {
		data.append((char)saut->value());
	} else {
		valeur = saut->value();
		data.append((char *)&valeur, 2);
	}
	
	return data;
}

IfKeyPage::IfKeyPage(QWidget *parent) : QWidget(parent)
{
	key_group = new QGroupBox(tr("Touches"));
	QGridLayout *key_layout = new QGridLayout(key_group);
	
	for(int i=0 ; i<DATA::key_names.size() ; ++i) {
		key_layout->addWidget(new QCheckBox(DATA::key_names.at(i), key_group), i%8, 1+i/8);
	}
	
	type = new QComboBox();
	type->addItem("-");
	type->addItem(tr("Touche appuyée une fois"));
	type->addItem(tr("Touche appuyée pour la première fois"));
	
	saut = new QSpinBox();
	saut->setRange(0, 255);

	QGridLayout *layout = new QGridLayout();
	layout->addWidget(key_group, 0, 0, 1, 2);
	layout->addWidget(new QLabel(tr("Type")), 1, 0);
	layout->addWidget(type, 1, 1);
	layout->addWidget(new QLabel(tr("Nombre d'octets à sauter")), 2, 0);
	layout->addWidget(saut, 2, 1);
	layout->setRowStretch(3, 1);
	
	setLayout(layout);
}

void IfKeyPage::fill(int opcode, QByteArray params)
{
	quint16 valeur;
	QList<QCheckBox *> keys = key_group->findChildren<QCheckBox *>();
	
	type->setCurrentIndex(opcode-0x30);
	
	memcpy(&valeur, params.mid(0,2), 2);
	
	for(int i=0 ; i<keys.size() ; ++i)
		keys.at(i)->setCheckState((valeur >> i) & 1 ? Qt::Checked : Qt::Unchecked);

	saut->setValue(params.at(2));
}

QByteArray IfKeyPage::save()
{
	QByteArray data;
	quint16 valeur = 0;
	QList<QCheckBox *> keys = key_group->findChildren<QCheckBox *>();
	
	// OPCODE
	qDebug() << type->currentIndex();
	data.append((char)(type->currentIndex()+0x30));
	
	for(int i=0 ; i<keys.size() ; ++i) {
		if(keys.at(i)->checkState() == Qt::Checked)
			valeur |= 1 << i;
	}
	
	data.append((char *)&valeur, 2);
	data.append((char)saut->value());
	
	return data;
}

IfCharPage::IfCharPage(QWidget *parent) : QWidget(parent)
{
	character = new QComboBox();
	character->addItems(DATA::char_names);
	int nbItems = character->count();
	for(int i=nbItems ; i<100 ; i++)
		character->addItem(QString("%1").arg(i));
	for(int i=100 ; i<254 ; i++)
		character->addItem(DATA::char_names.last());	
	character->addItem(tr("(Vide)"));
	character->addItem(tr("(Vide)"));
	
	type = new QComboBox();
	type->addItem(tr("Le personnage est l'un des trois combattants"));
	type->addItem(tr("Le personnage est dans l'équipe"));
	
	saut = new QSpinBox();
	saut->setRange(0, 255);

	QGridLayout *layout = new QGridLayout();
	layout->addWidget(new QLabel(tr("Personnage")), 0, 0);
	layout->addWidget(character, 0, 1);
	layout->addWidget(new QLabel(tr("Type")), 1, 0);
	layout->addWidget(type, 1, 1);
	layout->addWidget(new QLabel(tr("Nombre d'octets à sauter")), 2, 0);
	layout->addWidget(saut, 2, 1);
	layout->setRowStretch(3, 1);
	
	setLayout(layout);
}

void IfCharPage::fill(int opcode, QByteArray params)
{
	type->setCurrentIndex(opcode-0xCB);
	character->setCurrentIndex((quint8)params.at(0));
	saut->setValue((quint8)params.at(1));
}

QByteArray IfCharPage::save()
{
	QByteArray data;
	
	data.append((char)(type->currentIndex()+0xCB));
	data.append((char)character->currentIndex());
	data.append((char)saut->value());
	
	return data;
}

WaitPage::WaitPage(QWidget *parent) : QWidget(parent)
{
	time = new QSpinBox();
	time->setRange(0, 65535);

	QGridLayout *layout = new QGridLayout();
	layout->addWidget(new QLabel(tr("Nombre d'images")), 0, 0);
	layout->addWidget(time, 0, 1);
	layout->setRowStretch(1, 1);
	
	setLayout(layout);
}

void WaitPage::fill(int opcode, QByteArray params)
{
	quint16 valeur;
	memcpy(&valeur, params.mid(0,2), 2);
	time->setValue(valeur);
}

QByteArray WaitPage::save()
{
	QByteArray data;
	quint16 valeur;
	
	valeur = time->value();
	data.append((char *)&valeur, 2);
	
	return data;
}

NopPage::NopPage(QWidget *parent) : QWidget(parent)
{
	QVBoxLayout *layout = new QVBoxLayout();
	layout->addWidget(new QLabel(tr("Ne rien faire")));
	layout->addStretch();
	
	setLayout(layout);
}

void NopPage::fill(int, QByteArray)
{
}

QByteArray NopPage::save()
{
	return QByteArray();
}

OpPage::OpPage(QWidget *parent) : QWidget(parent)
{
	bank1 = new QSpinBox();
	bank1->setRange(1, 15);
	adresse = new QSpinBox();
	adresse->setRange(0, 255);
	
	QHBoxLayout *hlayout1 = new QHBoxLayout();
	hlayout1->addWidget(bank1);
	hlayout1->addWidget(adresse);
	
	operateur = new QComboBox();
	operateur->addItem(tr("Affectation"));
	operateur->addItem(tr("Addition"));
	operateur->addItem(tr("Soustraction"));
	operateur->addItem(tr("Multiplication"));
	operateur->addItem(tr("Division"));
	operateur->addItem(tr("Modulo"));
	operateur->addItem(tr("Et"));
	operateur->addItem(tr("Ou"));
	operateur->addItem(tr("Ou exclusif"));
	QObject::connect(operateur, SIGNAL(currentIndexChanged(int)), this, SLOT(setSaturatedEnabled(int)));
	
	bank2 = new QSpinBox();
	bank2->setRange(1, 15);
	bank2->hide();
	valeur2 = new QSpinBox();
	
	QHBoxLayout *hlayout2 = new QHBoxLayout();
	hlayout2->addWidget(bank2);
	hlayout2->addWidget(valeur2);
	
	QStringList strlist;
	strlist << tr("Valeur") << tr("Variable");
	
	QComboBox *type_valeur1 = new QComboBox();
	type_valeur1->addItem(strlist.at(1));
	type_valeur1->setEnabled(false);
	
	type_valeur2 = new QComboBox();
	type_valeur2->addItems(strlist);
	QObject::connect(type_valeur2, SIGNAL(currentIndexChanged(int)), this, SLOT(setType2(int)));
	
	range_valeur = new QComboBox();
	range_valeur->addItem(QString("0 -> 255"));
	range_valeur->addItem(QString("0 -> 65535"));
	QObject::connect(range_valeur, SIGNAL(currentIndexChanged(int)), this, SLOT(setRange_valeur(int)));

	saturated = new QCheckBox(tr("Saturé"));
	
	QGridLayout *layout = new QGridLayout();
	layout->addWidget(new QLabel(tr("Variable")), 0, 0);
	layout->addLayout(hlayout1, 0, 1);
	layout->addWidget(type_valeur1, 0, 2);
	layout->addWidget(new QLabel(tr("Opération")), 1, 0);
	layout->addWidget(operateur, 1, 1, 1, 2, Qt::AlignHCenter);
	layout->addWidget(new QLabel(tr("Valeur")), 2, 0);
	layout->addLayout(hlayout2, 2, 1);
	layout->addWidget(type_valeur2, 2, 2);
	layout->addWidget(new QLabel(tr("Étendue des valeurs")), 3, 0);
	layout->addWidget(range_valeur, 3, 1);
	layout->addWidget(saturated, 3, 2);
	layout->setRowStretch(4, 1);
	
	setLayout(layout);
}

void OpPage::setType2(int index)
{
	if(index==1) {
		bank2->show();
		valeur2->setRange(0, 255);
	} else {
		bank2->hide();
		setRange_valeur(range_valeur->currentIndex());
	}
}

void OpPage::setRange_valeur(int rangeIndex)
{
	if(rangeIndex==0 || rangeIndex==2) {
		adresse->setRange(0, 255);
		valeur2->setRange(0, 255);
	} else {
		adresse->setRange(0, 65535);
		valeur2->setRange(0, 65535);
	}
}

void OpPage::setSaturatedEnabled(int operateurIndex)
{
	saturated->setEnabled(operateurIndex==1 || operateurIndex==2);
}

void OpPage::fill(int opcode, QByteArray params)
{
	quint8 b2 = B2(params.at(0));
	quint16 valeur;
	
	bank1->setValue(B1(params.at(0)));
	bank2->setValue(b2);
	type_valeur2->setCurrentIndex(b2!=0);
	
	adresse->setValue((quint8)params.at(1));
	
	if(opcode==0x76 || opcode==0x78 || opcode==0x80 || opcode==0x85 || opcode==0x87
	   || opcode==0x89 || opcode==0x8B || opcode==0x8D || opcode==0x8F || opcode==0x91 || opcode==0x93) {
		range_valeur->setCurrentIndex(0);
		valeur2->setValue((quint8)params.at(2));
	}
	else {
		range_valeur->setCurrentIndex(1);
		memcpy(&valeur, params.mid(2,2), 2);
		valeur2->setValue(valeur);
	}
	
	if(opcode==0x80 || opcode==0x81)
		operateur->setCurrentIndex(0);
	if(opcode==0x76 || opcode==0x77 || opcode==0x85 || opcode==0x86)
		operateur->setCurrentIndex(1);
	if(opcode==0x78 || opcode==0x79 || opcode==0x87 || opcode==0x88)
		operateur->setCurrentIndex(2);
	if(opcode==0x89 || opcode==0x8A)
		operateur->setCurrentIndex(3);
	if(opcode==0x8B || opcode==0x8C)
		operateur->setCurrentIndex(4);
	if(opcode==0x8D || opcode==0x8E)
		operateur->setCurrentIndex(5);
	if(opcode==0x8F || opcode==0x90)
		operateur->setCurrentIndex(6);
	if(opcode==0x91 || opcode==0x92)
		operateur->setCurrentIndex(7);
	else
		operateur->setCurrentIndex(8);
	
	saturated->setChecked(opcode>=0x76 && opcode<=0x79);
}

QByteArray OpPage::save()
{
	QByteArray data;
	quint8 bank;
	quint16 valeur;
	
	switch(operateur->currentIndex()) {
	case 0:
		data.append((char)(range_valeur->currentIndex()==0 ? 0x80 : 0x81));
		break;
	case 1:
		if(saturated->isChecked())
			data.append((char)(range_valeur->currentIndex()==0 ? 0x76 : 0x77));
		else
			data.append((char)(range_valeur->currentIndex()==0 ? 0x85 : 0x86));
		break;
	case 2:
		if(saturated->isChecked())
			data.append((char)(range_valeur->currentIndex()==0 ? 0x78 : 0x79));
		else
			data.append((char)(range_valeur->currentIndex()==0 ? 0x87 : 0x88));
		break;
	case 3:
		data.append((char)(range_valeur->currentIndex()==0 ? 0x89 : 0x8A));
		break;
	case 4:
		data.append((char)(range_valeur->currentIndex()==0 ? 0x8B : 0x8C));
		break;
	case 5:
		data.append((char)(range_valeur->currentIndex()==0 ? 0x8D : 0x8E));
		break;
	case 6:
		data.append((char)(range_valeur->currentIndex()==0 ? 0x8F : 0x90));
		break;
	case 7:
		data.append((char)(range_valeur->currentIndex()==0 ? 0x91 : 0x92));
		break;
	default:
		data.append((char)(range_valeur->currentIndex()==0 ? 0x93 : 0x94));
		break;
	}
	
	bank = (bank1->value() & 0xF) << 4;
	bank |= bank2->isVisible() ? bank2->value() & 0xF : 0;
	data.append((char)bank);
	data.append((char)adresse->value());
	
	if(range_valeur->currentIndex()==0) {
		data.append((char)valeur2->value());
	}
	else {
		valeur = valeur2->value();
		data.append((char *)&valeur, 2);
	}
	
	return data;
}

Op1Page::Op1Page(QWidget *parent) : QWidget(parent)
{
	bank2 = new QSpinBox();
	bank2->setRange(1, 15);
	adresse = new QSpinBox();
	adresse->setRange(0, 255);
	
	QHBoxLayout *hlayout = new QHBoxLayout();
	hlayout->addWidget(bank2);
	hlayout->addWidget(adresse);
	
	QComboBox *type_valeur = new QComboBox();
	type_valeur->addItem(tr("Variable"));
	type_valeur->setEnabled(false);
	
	operateur = new QComboBox();
	operateur->addItem(tr("Incrémentation"));
	operateur->addItem(tr("Décrémentation"));
	
	range_valeur = new QComboBox();
	range_valeur->addItem(QString("0 -> 255"));
	range_valeur->addItem(QString("0 -> 65535"));

	saturated = new QCheckBox(tr("Saturé"));
	
	QGridLayout *layout = new QGridLayout();
	layout->addWidget(new QLabel(tr("Variable")), 0, 0);
	layout->addLayout(hlayout, 0, 1);
	layout->addWidget(type_valeur, 0, 2);
	layout->addWidget(new QLabel(tr("Opération")), 1, 0);
	layout->addWidget(operateur, 1, 1, 1, 2, Qt::AlignHCenter);
	layout->addWidget(new QLabel(tr("Étendue des valeurs")), 2, 0);
	layout->addWidget(range_valeur, 2, 1);
	layout->addWidget(saturated, 2, 2);
	layout->setRowStretch(3, 1);
	
	setLayout(layout);
}

void Op1Page::fill(int opcode, QByteArray params)
{
	bank2->setValue(B2(params.at(0)));
	adresse->setValue((quint8)params.at(1));
	
	range_valeur->setCurrentIndex(opcode==0x7B || opcode==0x7D || opcode==0x96 || opcode==0x98);
	operateur->setCurrentIndex(opcode==0x7C || opcode==0x7D || opcode==0x97 || opcode==0x98);
	saturated->setChecked(opcode==0x7A || opcode==0x7B || opcode==0x7C || opcode==0x7D);
}

QByteArray Op1Page::save()
{
	QByteArray data;
	
	if(operateur->currentIndex()==0) {
		if(saturated->isChecked())
			data.append((char)(range_valeur->currentIndex()==0 ? 0x7A : 0x7B));
		else
			data.append((char)(range_valeur->currentIndex()==0 ? 0x95 : 0x96));
	}
	else {
		if(saturated->isChecked())
			data.append((char)(range_valeur->currentIndex()==0 ? 0x7C : 0x7D));
		else
			data.append((char)(range_valeur->currentIndex()==0 ? 0x97 : 0x98));
	}
	
	data.append((char)(bank2->value() & 0xF));
	data.append((char)adresse->value());
	
	return data;
}


BgParamStatePage::BgParamStatePage(QWidget *parent) : QWidget(parent)
{
	show = new QRadioButton(tr("Afficher"), this);
	hide = new QRadioButton(tr("Cacher"), this);

	bank1 = new QSpinBox();
	bank1->setRange(1, 15);
	bank1->hide();
	parametre = new QSpinBox();
	parametre->setRange(0, 255);
	
	QHBoxLayout *hlayout1 = new QHBoxLayout();
	hlayout1->addWidget(bank1);
	hlayout1->addWidget(parametre);
	
	bank2 = new QSpinBox();
	bank2->setRange(1, 15);
	bank2->hide();
	etat = new QSpinBox();
	etat->setRange(0, 255);
	
	QHBoxLayout *hlayout2 = new QHBoxLayout();
	hlayout2->addWidget(bank2);
	hlayout2->addWidget(etat);
	
	QStringList strlist;
	strlist << tr("Valeur") << tr("Variable");
	
	type_parametre = new QComboBox();
	type_parametre->addItems(strlist);
	QObject::connect(type_parametre, SIGNAL(currentIndexChanged(int)), this, SLOT(setType1(int)));
	
	type_etat = new QComboBox();
	type_etat->addItems(strlist);
	QObject::connect(type_etat, SIGNAL(currentIndexChanged(int)), this, SLOT(setType2(int)));
	
	QGridLayout *layout = new QGridLayout();
	layout->addWidget(show, 0, 1, Qt::AlignRight);
	layout->addWidget(hide, 0, 2, Qt::AlignLeft);
	layout->addWidget(new QLabel(tr("Première valeur")), 1, 0);
	layout->addLayout(hlayout1, 1, 1);
	layout->addWidget(type_parametre, 1, 2);
	layout->addWidget(new QLabel(tr("Seconde valeur")), 2, 0);
	layout->addLayout(hlayout2, 2, 1);
	layout->addWidget(type_etat, 2, 2);
	layout->setRowStretch(3, 1);
	
	setLayout(layout);
}

void BgParamStatePage::setType1(int index)
{
	bank1->setVisible(index==1);
}

void BgParamStatePage::setType2(int index)
{
	bank2->setVisible(index==1);
}

void BgParamStatePage::fill(int opcode, QByteArray params)
{
	/* quint8 b2 = B2(params.at(0));
	quint16 valeur;
	
	bank1->setValue(B1(params.at(0)));
	bank2->setValue(b2);
	type_valeur2->setCurrentIndex(b2!=0);
	
	valeur1->setValue((quint8)params.at(1));
	
	if(opcode==0x76 || opcode==0x78 || opcode==0x80 || opcode==0x85 || opcode==0x87
	|| opcode==0x89 || opcode==0x8B || opcode==0x8D || opcode==0x8F || opcode==0x91 || opcode==0x93) {
		range_valeur->setCurrentIndex(0);
		valeur2->setValue((quint8)params.at(2));
	}
	else {
		range_valeur->setCurrentIndex(1);
		memcpy(&valeur, params.mid(2,2), 2);
		valeur2->setValue(valeur);
	}
	
	if(opcode==0x80 || opcode==0x81)
		operateur->setCurrentIndex(0);
	if(opcode==0x76 || opcode==0x77 || opcode==0x85 || opcode==0x86)
		operateur->setCurrentIndex(1);
	if(opcode==0x78 || opcode==0x79 || opcode==0x87 || opcode==0x88)
		operateur->setCurrentIndex(2);
	if(opcode==0x89 || opcode==0x8A)
		operateur->setCurrentIndex(3);
	if(opcode==0x8B || opcode==0x8C)
		operateur->setCurrentIndex(4);
	if(opcode==0x8D || opcode==0x8E)
		operateur->setCurrentIndex(5);
	if(opcode==0x8F || opcode==0x90)
		operateur->setCurrentIndex(6);
	if(opcode==0x91 || opcode==0x92)
		operateur->setCurrentIndex(7);
	if(opcode==0x93 || opcode==0x94)
		operateur->setCurrentIndex(8);

	saturated->setChecked(opcode>=0x76 && opcode<=0x79); */
}

QByteArray BgParamStatePage::save()
{
	/* QByteArray data;
	quint8 bank;
	quint16 valeur;
	
	switch(operateur->currentIndex()) {
		case 0:
			data.append((char)(range_valeur->currentIndex()==0 ? 0x80 : 0x81));
		break;
		case 1:
			if(saturated->isChecked())
				data.append((char)(range_valeur->currentIndex()==0 ? 0x76 : 0x77));
			else
				data.append((char)(range_valeur->currentIndex()==0 ? 0x85 : 0x86));
		break;
		case 2:
			if(saturated->isChecked())
				data.append((char)(range_valeur->currentIndex()==0 ? 0x78 : 0x79));
			else
				data.append((char)(range_valeur->currentIndex()==0 ? 0x87 : 0x88));
		break;
		case 3:
			data.append((char)(range_valeur->currentIndex()==0 ? 0x89 : 0x8A));
		break;
		case 4:
			data.append((char)(range_valeur->currentIndex()==0 ? 0x8B : 0x8C));
		break;
		case 5:
			data.append((char)(range_valeur->currentIndex()==0 ? 0x8D : 0x8E));
		break;
		case 6:
			data.append((char)(range_valeur->currentIndex()==0 ? 0x8F : 0x90));
		break;
		case 7:
			data.append((char)(range_valeur->currentIndex()==0 ? 0x91 : 0x92));
		break;
		case 8:
			data.append((char)(range_valeur->currentIndex()==0 ? 0x93 : 0x94));
		break;
	}
	
	bank = bank1->isVisible() ? (bank1->value() & 0xF) << 4 : 0;
	bank |= bank2->isVisible() ? bank2->value() & 0xF : 0;
	data.append((char)bank);
	data.append((char)valeur1->value());
	
	if(range_valeur->currentIndex()==0) {
		data.append((char)valeur2->value());
	}
	else {
		valeur = valeur2->value();
		data.append((char *)&valeur, 2);
	}
	
	return data; */
}

#ifndef DEF_SE_DIALOGS
#define DEF_SE_DIALOGS

#include <QtGui>
#include "Var.h"
#include "parametres.h"

class GotoPage : public QWidget
{
	Q_OBJECT
public:
	GotoPage(QWidget *parent = 0);
	void fill(int, QByteArray);
	QByteArray save();
private:
	QSpinBox *saut;
	QComboBox *sens;
	QComboBox *range;
private slots:
	void setRange(int);
};

class ReturnPage : public QWidget
{
public:
	ReturnPage(QWidget *parent = 0);
	void fill(int, QByteArray);
	QByteArray save();
private:
	QGroupBox *callback;
	QSpinBox *priority;
	QSpinBox *script;
};

class ExecPage : public QWidget
{
public:
	ExecPage(QWidget *parent = 0);
	void fill(int, QByteArray);
	QByteArray save();
private:
	QComboBox *type;
	QComboBox *group;
	QSpinBox *priority;
	QSpinBox *script;
};

class ExecCharPage : public QWidget
{
public:
	ExecCharPage(QWidget *parent = 0);
	void fill(int, QByteArray);
	QByteArray save();
private:
	QComboBox *type;
	QSpinBox *character;
	QSpinBox *priority;
	QSpinBox *script;
};

class IfPage : public QWidget
{
	Q_OBJECT
public:
	IfPage(QWidget *parent = 0);
	void fill(int, QByteArray);
	QByteArray save();
private:
	QSpinBox *bank1;
	QSpinBox *valeur1;
	QComboBox *type_valeur1;
	QComboBox *operateur;
	QSpinBox *bank2;
	QSpinBox *valeur2;
	QComboBox *type_valeur2;
	QComboBox *range_valeurs;
	QComboBox *range_saut;
	QSpinBox *saut;
private slots:
	void setType1(int);
	void setType2(int);
	void setRange_valeurs(int);
	void setRange_saut(int);
};

class IfKeyPage : public QWidget
{
public:
	IfKeyPage(QWidget *parent = 0);
	void fill(int, QByteArray);
	QByteArray save();
private:
	QGroupBox *key_group;
	QComboBox *type;
	QSpinBox *saut;
};

class IfCharPage : public QWidget
{
public:
	IfCharPage(QWidget *parent = 0);
	void fill(int, QByteArray);
	QByteArray save();
private:
	QComboBox *character;
	QComboBox *type;
	QSpinBox *saut;
};

class WaitPage : public QWidget
{
public:
	WaitPage(QWidget *parent = 0);
	void fill(int, QByteArray);
	QByteArray save();
private:
	QSpinBox *time;
};

class NopPage : public QWidget
{
public:
	NopPage(QWidget *parent = 0);
	void fill(int, QByteArray);
	QByteArray save();
};

class OpPage : public QWidget
{
	Q_OBJECT
public:
	OpPage(QWidget *parent = 0);
	void fill(int, QByteArray);
	QByteArray save();
private:
	QSpinBox *bank1;
	QSpinBox *adresse;
	QComboBox *operateur;
	QSpinBox *bank2;
	QSpinBox *valeur2;
	QComboBox *type_valeur2;
	QComboBox *range_valeur;
	QCheckBox *saturated;
private slots:
	void setType2(int);
	void setRange_valeur(int);
	void setSaturatedEnabled(int);
};

class Op1Page : public QWidget
{
public:
	Op1Page(QWidget *parent = 0);
	void fill(int, QByteArray);
	QByteArray save();
private:
	QSpinBox *bank2;
	QSpinBox *adresse;
	QComboBox *operateur;
	QComboBox *range_valeur;
	QCheckBox *saturated;
};


class BgParamStatePage : public QWidget
{
	Q_OBJECT
public:
	BgParamStatePage(QWidget *parent = 0);
	void fill(int, QByteArray);
	QByteArray save();
private:
	QSpinBox *bank1;
	QSpinBox *parametre;
	QComboBox *type_parametre;
	QSpinBox *bank2;
	QSpinBox *etat;
	QComboBox *type_etat;
	QRadioButton *show;
	QRadioButton *hide;
private slots:
	void setType1(int);
	void setType2(int);
};

#endif

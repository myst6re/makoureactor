#ifndef VARORVALUEWIDGET_H
#define VARORVALUEWIDGET_H

#include <QtGui>

class VarOrValueWidget : public QWidget
{
	Q_OBJECT
public:
	explicit VarOrValueWidget(QWidget *parent = 0);
	int value() const;
	void setValue(int value);
	void var(quint8 &bank, quint8 &adress) const;
	void setVar(quint8 bank, quint8 adress);
	bool isValue() const;
	void setIsValue(bool isValue);
	bool isLongValueType() const;
	void setLongValueType(bool longValueType);
	bool isSignedValueType() const;
	void setSignedValueType(bool signedValueType);
	bool isOnlyVar() const;
	void setOnlyVar(bool onlyVar);
signals:
	void changed();
private:
	void updateValueRange();
	bool _longValueType, _signedValueType;
	QStackedLayout *varOrValuelayout;
	QComboBox *typeSelect;
	QSpinBox *_value, *_bank, *_adress;
};

#endif // VARORVALUEWIDGET_H

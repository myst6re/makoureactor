#ifndef OPERATIONSMANAGER_H
#define OPERATIONSMANAGER_H

#include <QtGui>
#include "core/field/FieldArchive.h"

class OperationsManager : public QDialog
{
	Q_OBJECT
public:
	enum Operation {
		CleanUnusedTexts = 0x01,
		RemoveTexts = 0x02,
		RemoveBattles = 0x04,
		CleanModelLoaderPC = 0x08,
		RemoveUnusedSectionPC = 0x10
	};
	Q_DECLARE_FLAGS(Operations, Operation)

	explicit OperationsManager(bool isPC, QWidget *parent = 0);
	Operations selectedOperations() const;
private slots:
	void updateApplyButton();
private:
	void addOperation(Operation op, const QString &description);
	QMap<Operation, QCheckBox *> _operations;
	QAbstractButton *applyButton;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(OperationsManager::Operations)

#endif // OPERATIONSMANAGER_H

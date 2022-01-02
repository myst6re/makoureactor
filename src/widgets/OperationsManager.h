/****************************************************************************
 ** Makou Reactor Final Fantasy VII Field Script Editor
 ** Copyright (C) 2009-2022 Arzel Jérôme <myst6re@gmail.com>
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
#pragma once

#include <QtWidgets>
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
		RemoveUnusedSectionPC = 0x10,
		RepairBackgroundsPC = 0x20,
		AutosizeTextWindows = 0x40
	};
	Q_DECLARE_FLAGS(Operations, Operation)

	explicit OperationsManager(bool isPC, QWidget *parent = nullptr);
	Operations selectedOperations() const;
private slots:
	void updateApplyButton();
private:
	void addOperation(Operation op, const QString &description);
	QMap<Operation, QCheckBox *> _operations;
	QAbstractButton *applyButton;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(OperationsManager::Operations)

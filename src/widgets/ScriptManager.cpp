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
#include "ScriptManager.h"
#include "core/Config.h"

ScriptManager::ScriptManager(QWidget *parent) :
    QWidget(parent), _field(nullptr)
{
	_groupScriptList = new GrpScriptList(this);
	_groupScriptList->setMinimumHeight(176);

	_scriptList = new ScriptList(this);
	_scriptList->setMaximumWidth(
	    _scriptList->fontMetrics().boundingRect(QString(20, 'M')).width());
	_scriptList->setMinimumWidth(
	    _scriptList->fontMetrics().boundingRect(QString(10, 'M')).width());
	_scriptList->setMinimumHeight(88);

	_opcodeList = new OpcodeList(this);

	_compileScriptLabel = new QLabel(this);
	_compileScriptLabel->hide();
	QPalette pal = _compileScriptLabel->palette();
	pal.setColor(QPalette::Active, QPalette::WindowText, Qt::red);
	pal.setColor(QPalette::Inactive, QPalette::WindowText, Qt::red);
	_compileScriptLabel->setPalette(pal);
	_compileScriptIcon = new QLabel(this);
	_compileScriptIcon->setPixmap(QIcon::fromTheme(QStringLiteral("dialog-critical")).pixmap(16));
	_compileScriptIcon->hide();
	
	QVBoxLayout *layoutGroupScript = new QVBoxLayout;
	layoutGroupScript->addWidget(_groupScriptList);
	layoutGroupScript->addWidget(_groupScriptList->helpWidget());
	layoutGroupScript->setSpacing(2);
	layoutGroupScript->setContentsMargins(QMargins());

	QHBoxLayout *compileLayout = new QHBoxLayout;
	compileLayout->addWidget(_compileScriptIcon);
	compileLayout->addWidget(_compileScriptLabel, 1);
	compileLayout->setContentsMargins(QMargins());

	QVBoxLayout *layoutScript = new QVBoxLayout;
	layoutScript->addWidget(_opcodeList);
	layoutScript->addLayout(compileLayout);
	layoutScript->setSpacing(2);
	layoutScript->setContentsMargins(QMargins());

	QGridLayout *contentLayout = new QGridLayout(this);
	contentLayout->addWidget(_groupScriptList->toolBar(), 0, 0);
	contentLayout->addLayout(layoutGroupScript, 1, 0);
	contentLayout->addWidget(_scriptList, 1, 1);
	contentLayout->addWidget(_opcodeList->toolBar(), 0, 2);
	contentLayout->addLayout(layoutScript, 1, 2);
	contentLayout->setColumnStretch(0, 2);
	contentLayout->setColumnStretch(1, 1);
	contentLayout->setColumnStretch(2, 9);

	connect(_groupScriptList, &GrpScriptList::changed, this, &ScriptManager::changed);
	connect(_groupScriptList, &GrpScriptList::itemSelectionChanged, this, &ScriptManager::fillScripts);

	connect(_scriptList, &ScriptList::itemSelectionChanged, this, &ScriptManager::fillOpcodes);

	connect(_opcodeList, &OpcodeList::changed, this, &ScriptManager::changed);
	connect(_opcodeList, &OpcodeList::changed, this, &ScriptManager::refresh);
	connect(_opcodeList, qOverload<int>(&OpcodeList::editText), this, &ScriptManager::editText);
	connect(_opcodeList, &OpcodeList::changed, this, &ScriptManager::compile);
	connect(_opcodeList, &OpcodeList::gotoScript, this, &ScriptManager::gotoScript);
	connect(_opcodeList, &OpcodeList::gotoField, this, &ScriptManager::gotoField);
	connect(_opcodeList, qOverload<int>(&OpcodeList::searchOpcode), this, &ScriptManager::searchOpcode);

	_groupScriptList->toolBar()->setVisible(Config::value("grpToolbarVisible", true).toBool());
	_opcodeList->toolBar()->setVisible(Config::value("scriptToolbarVisible", true).toBool());

	addActions(QList<QAction *>()
	           << _groupScriptList->toolBar()->toggleViewAction()
	           << _opcodeList->toolBar()->toggleViewAction());
}

void ScriptManager::saveConfig()
{
	Config::setValue("grpToolbarVisible", !_groupScriptList->toolBar()->isHidden());
	Config::setValue("scriptToolbarVisible", !_opcodeList->toolBar()->isHidden());
}

void ScriptManager::removeCopiedReferences()
{
	_groupScriptList->clearCopiedGroups();
}

void ScriptManager::clear()
{
	_groupScriptList->blockSignals(true);
	_scriptList->blockSignals(true);
	_opcodeList->blockSignals(true);

	_groupScriptList->clear();
	_groupScriptList->enableActions(false);
	_scriptList->clear();
	_opcodeList->clear();
	_field = nullptr;

	_groupScriptList->blockSignals(false);
	_scriptList->blockSignals(false);
	_opcodeList->blockSignals(false);
}

void ScriptManager::fill(Field *field)
{
	clear();

	_field = field;
	if (_field == nullptr || _field->scriptsAndTexts() == nullptr) {
		return;
	}

	if (_field->scriptsAndTexts()->isOpen()) {
		_groupScriptList->blockSignals(true);
		_groupScriptList->fill(_field->scriptsAndTexts());
		_groupScriptList->blockSignals(false);
		_groupScriptList->setEnabled(true);
	}
}

void ScriptManager::fillScripts()
{
	if (_field == nullptr) {
		return;
	}

	GrpScript *currentGrpScript = _groupScriptList->currentGrpScript();
	if (currentGrpScript) {
		_scriptList->blockSignals(true);
		_scriptList->fill(currentGrpScript);
		_scriptList->blockSignals(false);
		// Select first entry
		_scriptList->setCurrentRow(0);
	}
	emit groupScriptCurrentChanged(_groupScriptList->selectedID());
}

void ScriptManager::refreshOpcode(int groupID, int scriptID, int opcodeID)
{
	if (_field == nullptr || groupID != _groupScriptList->selectedID()
	    || scriptID != _scriptList->selectedID()) {
		return;
	}

	if (_scriptList->currentScript()) {
		_opcodeList->refreshOpcode(opcodeID);
	}
}

void ScriptManager::fillOpcodes()
{
	if (_field == nullptr) {
		return;
	}

	Script *currentScript = _scriptList->currentScript();
	if (currentScript) {
		_opcodeList->blockSignals(true);
		_opcodeList->fill(_field, _groupScriptList->currentGrpScript(),
		                 currentScript);
		_opcodeList->blockSignals(false);
		_opcodeList->setIsInit(_scriptList->selectedID() == 0);
		// Scroll to top
		_opcodeList->scroll(0, false);
	}
}

void ScriptManager::compile()
{
	Script *currentScript = _scriptList->currentScript();
	if (!currentScript) {
		return;
	}

	int opcodeID;
	QString errorStr;

	if (!currentScript->compile(opcodeID, errorStr)) {
		_compileScriptLabel->setText(tr("Error on line %1 : %2")
		                             .arg(opcodeID + 1)
		                             .arg(errorStr));
		_compileScriptLabel->show();
		_compileScriptIcon->show();
		_opcodeList->setErrorLine(opcodeID);
	} else {
		_compileScriptLabel->hide();
		_compileScriptIcon->hide();
		_opcodeList->setErrorLine(-1);
	}
}

void ScriptManager::refresh()
{
	_groupScriptList->localeRefresh();
	_scriptList->localeRefresh();
}

void ScriptManager::gotoScript(int grpScriptID, int scriptID)
{
	_groupScriptList->scroll(grpScriptID, false);
	_scriptList->scroll(scriptID, false);
}

void ScriptManager::gotoOpcode(int grpScriptID, int scriptID, int opcodeID)
{
	gotoScript(grpScriptID, scriptID);
	_opcodeList->scroll(opcodeID);
}

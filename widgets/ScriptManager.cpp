#include "ScriptManager.h"
#include "core/Config.h"

ScriptManager::ScriptManager(QWidget *parent) :
    QWidget(parent), _field(0)
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
	_compileScriptIcon->setPixmap(QApplication::style()->standardIcon(QStyle::SP_MessageBoxCritical).pixmap(16));
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

	connect(_groupScriptList, SIGNAL(changed()), SIGNAL(changed()));
	connect(_groupScriptList, SIGNAL(itemSelectionChanged()), SLOT(fillScripts()));

	connect(_scriptList, SIGNAL(itemSelectionChanged()), SLOT(fillOpcodes()));

	connect(_opcodeList, SIGNAL(changed()), SIGNAL(changed()));
	connect(_opcodeList, SIGNAL(changed()), SLOT(refresh()));
	connect(_opcodeList, SIGNAL(editText(int)), SIGNAL(editText(int)));
	connect(_opcodeList, SIGNAL(changed()), SLOT(compile()));
	connect(_opcodeList, SIGNAL(gotoScript(int,int)), SLOT(gotoScript(int,int)));
	connect(_opcodeList, SIGNAL(gotoField(int)), SIGNAL(gotoField(int)));
	connect(_opcodeList, SIGNAL(searchOpcode(int)), SIGNAL(searchOpcode(int)));

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
	_field = 0;

	_groupScriptList->blockSignals(false);
	_scriptList->blockSignals(false);
	_opcodeList->blockSignals(false);
}

void ScriptManager::fill(Field *field)
{
	clear();

	_field = field;
	if (!_field) {
		return;
	}

	Section1File *scriptsAndTexts = _field->scriptsAndTexts();

	if (scriptsAndTexts->isOpen()) {
		_groupScriptList->blockSignals(true);
		_groupScriptList->fill(scriptsAndTexts);
		_groupScriptList->blockSignals(false);
		_groupScriptList->setEnabled(true);
	}
}

void ScriptManager::fillScripts()
{
	if (!_field) {
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

void ScriptManager::fillOpcodes()
{
	if (!_field) {
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

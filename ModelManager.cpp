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
#include "ModelManager.h"

ModelManager::ModelManager(Field *field, const QGLWidget *shareWidget, QWidget *parent)
	: QDialog(parent, Qt::Dialog | Qt::WindowCloseButtonHint), fieldModelLoader(field->getFieldModelLoaderPC()), page_filled(false)
{
	field_model_unknown = fieldModelLoader->model_unknown;
	field_model_nameChar = fieldModelLoader->model_nameChar;
	field_model_nameHRC = fieldModelLoader->model_nameHRC;
	field_model_typeHRC = fieldModelLoader->model_typeHRC;
	field_model_anims = fieldModelLoader->model_anims;
	field_colors = fieldModelLoader->colors;

	setWindowTitle(tr("Modèles 3D"));
	QFont font;
	font.setPointSize(8);

	typeHRC = new QSpinBox();
	typeHRC->setRange(0, 65535);
	typeHRC->setValue(fieldModelLoader->typeHRC);

	QToolBar *toolBar1 = new QToolBar();
	toolBar1->setIconSize(QSize(14,14));
	toolBar1->addAction(QIcon(":/images/plus.png"), QString(), this, SLOT(add_HRC()));
	toolBar1->addAction(QIcon(":/images/minus.png"), QString(), this, SLOT(del_HRC()));
	toolBar1->addAction(QIcon(":/images/up.png"), QString(), this, SLOT(up_HRC()));
	toolBar1->addAction(QIcon(":/images/down.png"), QString(), this, SLOT(down_HRC()));

	models = new QTreeWidget();
	models->setColumnCount(1);
	models->setIndentation(0);
	models->setHeaderLabel(tr("Modèles 3D"));
	models->setFixedWidth(120);
	QTreeWidgetItem *item;
	int size = field_model_nameHRC.size();
	for(int i=0 ; i<size ; ++i)
	{
		item = new QTreeWidgetItem(QStringList(field_model_nameHRC.at(i)));
		item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsEditable);
		models->addTopLevelItem(item);
	}

	model_frame = new QFrame();
	model_frame->setFrameShape(QFrame::StyledPanel);

	model_name = new QLineEdit();
	model_name->setMaxLength(256-field->getName().size());
	model_unknown = new QSpinBox();
	model_unknown->setRange(0, 65535);
	model_typeHRC = new QSpinBox();
	model_typeHRC->setRange(0, 4096);
	model_colorDisplay = new ColorDisplay();

	toolBar2 = new QToolBar();
	toolBar2->setIconSize(QSize(14,14));
	toolBar2->addAction(QIcon(":/images/plus.png"), QString(), this, SLOT(add_anim()));
	toolBar2->addAction(QIcon(":/images/minus.png"), QString(), this, SLOT(del_anim()));
	toolBar2->addAction(QIcon(":/images/up.png"), QString(), this, SLOT(up_anim()));
	toolBar2->addAction(QIcon(":/images/down.png"), QString(), this, SLOT(down_anim()));

	model_anims = new QTreeWidget();
	model_anims->setColumnCount(1);
	model_anims->setIndentation(0);
	model_anims->setHeaderLabel(tr("Animations"));
	model_anims->setFixedWidth(120);

	QWidget *modelWidget;
	if(Config::value("OpenGL", true).toBool()) {
		model_preview = new FieldModel(0, shareWidget);
		modelWidget = model_preview;
		model_preview->setFixedSize(304, 214);
	} else {
		model_preview = 0;
		modelWidget = new QWidget(this);
	}

	QGridLayout *frameLayout = new QGridLayout();
	frameLayout->addWidget(new QLabel(tr("Nom (non utilisé)")), 0, 0);
	frameLayout->addWidget(model_name, 0, 1);
	frameLayout->addWidget(new QLabel(tr("Inconnu")), 1, 0);
	frameLayout->addWidget(model_unknown, 1, 1);
	frameLayout->addWidget(new QLabel(tr("Taille modèle")), 2, 0);
	frameLayout->addWidget(model_typeHRC, 2, 1);
	frameLayout->addWidget(new QLabel(tr("Lumière")), 3, 0);
	frameLayout->addWidget(model_colorDisplay, 3, 1);
	frameLayout->addWidget(toolBar2, 0, 2);
	frameLayout->addWidget(model_anims, 1, 2, 4, 1);
	frameLayout->addWidget(modelWidget, 0, 3, 5, 1);
	model_frame->setLayout(frameLayout);

	QPushButton *buttonOK = new QPushButton(tr("OK"));
	QPushButton *buttonCancel = new QPushButton(tr("Annuler"));
	QHBoxLayout *buttonLayout = new QHBoxLayout();
	buttonLayout->addStretch();
	buttonLayout->addWidget(buttonOK);
	buttonLayout->addWidget(buttonCancel);

	QGridLayout *layout = new QGridLayout();
	layout->addWidget(toolBar1, 0, 0);
	layout->addWidget(new QLabel(tr("Taille modèles (non utilisé)")), 0, 1);
	layout->addWidget(typeHRC, 0, 2);
	layout->addWidget(models, 1, 0);
	layout->addWidget(model_frame, 1, 1, 1, 4);
	layout->addLayout(buttonLayout, 2, 0, 1, 5);

	setLayout(layout);
	adjustSize();

	connect(models, SIGNAL(currentItemChanged(QTreeWidgetItem*,QTreeWidgetItem*)), SLOT(show_HRC_infos(QTreeWidgetItem*,QTreeWidgetItem*)));
	connect(models, SIGNAL(doubleClicked(QModelIndex)), models, SLOT(edit(QModelIndex)));
	connect(models, SIGNAL(itemChanged(QTreeWidgetItem *, int)), SLOT(renameOK_HRC(QTreeWidgetItem *)));

	connect(model_anims, SIGNAL(currentItemChanged(QTreeWidgetItem*,QTreeWidgetItem*)), SLOT(show_model(QTreeWidgetItem*)));
	connect(model_anims, SIGNAL(doubleClicked(QModelIndex)), model_anims, SLOT(edit(QModelIndex)));
	connect(model_anims, SIGNAL(itemChanged(QTreeWidgetItem *, int)), SLOT(renameOK_anim(QTreeWidgetItem *)));

	connect(buttonOK, SIGNAL(released()), SLOT(accept()));
	connect(buttonCancel, SIGNAL(released()), SLOT(close()));

	models->setCurrentItem(models->topLevelItem(0));
	models->setFocus();
}

void ModelManager::show_HRC_infos(QTreeWidgetItem *item, QTreeWidgetItem *previous)
{
	save_page(previous);
	if(item==NULL) {
		model_frame->setEnabled(false);
		return;
	}
	model_frame->setEnabled(true);
	int row = models->indexOfTopLevelItem(item);
	model_anims->clear();

	if(row < 0 || row >= field_model_anims.size())	return;

	const QStringList &animNames = field_model_anims.at(row);
	int size = animNames.size();
	for(int i=0 ; i<size ; ++i)
	{
		item = new QTreeWidgetItem(QStringList(animNames.at(i)));
		item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsEditable);
		model_anims->addTopLevelItem(item);
	}

	model_name->setText(field_model_nameChar.at(row));
	model_unknown->setValue(field_model_unknown.at(row));
	model_typeHRC->setValue(field_model_typeHRC.at(row));
	model_colorDisplay->setColors(field_colors.at(row));
	if(model_anims->topLevelItemCount()>0)
		model_anims->setCurrentItem(model_anims->topLevelItem(0));
	page_filled = true;
}

void ModelManager::add_HRC()
{
	QString hrc("    .HRC");
	QTreeWidgetItem *item = models->currentItem(), *newItem;

	if(!Data::charlgp_listPos.isEmpty()) {
		QDialog dialog(this, Qt::Dialog | Qt::WindowCloseButtonHint);
		dialog.setWindowTitle(tr("Ajouter un modèle 3D"));

		QComboBox list(&dialog);
		list.setEditable(true);
		QStringList files;
		QString file;

		QHashIterator<QString, int> i(Data::charlgp_listPos);
		while (i.hasNext()) {
			i.next();
			file = i.key();
			if(file.endsWith(QString(".hrc"), Qt::CaseInsensitive)) {
				files.append(file);
			}
		}
		qSort(files);
		list.addItems(files);

		QPushButton OKButton(tr("OK"), &dialog);

		QVBoxLayout layout(&dialog);
		layout.addWidget(&list);
		layout.addWidget(&OKButton);

		connect(&list, SIGNAL(currentIndexChanged(QString)), SLOT(modifyHRC(QString)));
		connect(&OKButton, SIGNAL(released()), &dialog, SLOT(accept()));

		if(dialog.exec()==QDialog::Accepted) {
			hrc = list.currentText();
		}
		else {
			if(model_preview) {
				newItem = model_anims->currentItem();
				if(item!=NULL && newItem!=NULL)
					model_preview->load(item->text(0), newItem->text(0), true);
				else
					model_preview->clear();
			}
			return;
		}
	}

	if(model_preview)	model_preview->clear();

	newItem = new QTreeWidgetItem(QStringList(hrc));
	QList<QRgb> color;
	if(!field_colors.isEmpty())
		color = field_colors.first();
	if(item==NULL) {
		models->addTopLevelItem(newItem);

		field_model_unknown.append((quint16)0);
		field_model_nameChar.append(QString(".char"));
		field_model_nameHRC.append(QString());
		field_model_typeHRC.append(typeHRC->value());
		field_model_anims.append(QStringList());
		field_colors.append(color);
	}
	else {
		int row = models->indexOfTopLevelItem(item);
		models->insertTopLevelItem(row+1, newItem);

		field_model_unknown.insert(row+1, (quint16)0);
		field_model_nameChar.insert(row+1, QString(".char"));
		field_model_nameHRC.insert(row+1, QString());
		field_model_typeHRC.insert(row+1, typeHRC->value());
		field_model_anims.insert(row+1, QStringList());
		field_colors.insert(row+1, color);
	}
	newItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsEditable);
	page_filled = false;
	models->setCurrentItem(newItem);
	page_filled = true;
	if(Data::charlgp_listPos.isEmpty())
		models->editItem(newItem);
}

void ModelManager::del_HRC()
{
	QTreeWidgetItem *item = models->currentItem();
	if(item==NULL) return;

	int row = models->indexOfTopLevelItem(item);
	field_model_unknown.removeAt(row);
	field_model_nameChar.removeAt(row);
	field_model_nameHRC.removeAt(row);
	field_model_typeHRC.removeAt(row);
	field_model_anims.removeAt(row);
	field_colors.removeAt(row);

	page_filled = false;
	delete item;
	page_filled = true;
}

void ModelManager::up_HRC()
{
	QTreeWidgetItem *item = models->currentItem();
	if(item==NULL) return;
	int row = models->indexOfTopLevelItem(item);
	if(row<=0)	return;

	QTreeWidgetItem *itemAbove = models->itemAbove(item);

	QString savText = itemAbove->text(0);
	itemAbove->setText(0, item->text(0));
	item->setText(0, savText);

	field_model_unknown.swap(row, row-1);
	field_model_nameChar.swap(row, row-1);
	field_model_typeHRC.swap(row, row-1);
	field_model_anims.swap(row, row-1);
	field_colors.swap(row, row-1);

	page_filled = false;
	models->setCurrentItem(itemAbove);
	page_filled = true;
}

void ModelManager::down_HRC()
{
	QTreeWidgetItem *item = models->currentItem();
	if(item==NULL) return;
	int row = models->indexOfTopLevelItem(item);
	if(row>=models->topLevelItemCount()-1)	return;

	QTreeWidgetItem *itemBelow = models->itemBelow(item);

	QString savText = itemBelow->text(0);
	itemBelow->setText(0, item->text(0));
	item->setText(0, savText);

	field_model_unknown.swap(row, row+1);
	field_model_nameChar.swap(row, row+1);
	field_model_typeHRC.swap(row, row+1);
	field_model_anims.swap(row, row+1);
	field_colors.swap(row, row+1);

	page_filled = false;
	models->setCurrentItem(itemBelow);
	page_filled = true;
}

void ModelManager::renameOK_HRC(QTreeWidgetItem *item)
{
	QString text = item->text(0).left(8);
	int row = models->indexOfTopLevelItem(item);
	item->setText(0, text);
	field_model_nameHRC.replace(row, text);
}

void ModelManager::modifyHRC(const QString &hrc)
{
	if(model_preview) {
		model_preview->load(hrc, "aafe", false);
		switch(model_preview->nb_bones()) {
		case 5:
			model_preview->load(hrc, "avhd", false);
			break;
		case 9:
			model_preview->load(hrc, "geaf", false);
			break;
		case 11:
			model_preview->load(hrc, "bdfe", false);
			break;
		case 12:
			model_preview->load(hrc, "atcf", false);
			break;
		case 13:
			model_preview->load(hrc, "bria", false);
			break;
		case 14:
			model_preview->load(hrc, "fgad", false);
			break;
		case 15:
			model_preview->load(hrc, "gcad", false);
			break;
		case 17:
			model_preview->load(hrc, "hkga", false);
			break;
		case 18:
			model_preview->load(hrc, "gsia", false);
			break;
		case 20:
			model_preview->load(hrc, "FZGB", false);
			break;
		case 23:
			model_preview->load(hrc, "anhd", false);
			break;
		case 24:
			model_preview->load(hrc, "abcd", false);
			break;
		case 25:
			model_preview->load(hrc, "afdf", false);
			break;
		case 26:
			model_preview->load(hrc, "hlfb", false);
			break;
		case 27:
			model_preview->load(hrc, "fhhb", false);
			break;
		case 28:
			model_preview->load(hrc, "aeha", false);
			break;
		case 29:
			model_preview->load(hrc, "aeae", false);
			break;
		}
	}
}

void ModelManager::add_anim()
{
	QTreeWidgetItem *item = model_anims->currentItem(), *newItem;
	QString a;
	if(!Data::charlgp_listPos.isEmpty())
	{
		toolBar2->setEnabled(false);
		QDialog dialog(this, Qt::Dialog | Qt::WindowCloseButtonHint);
		dialog.setWindowTitle(tr("Ajouter un modèle 3D"));

		QComboBox list(&dialog);
		list.setEditable(true);
		list.setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLengthWithIcon);

		Data::charlgp_loadAnimBoneCount();

		QStringList files;
		QString file;
		QHashIterator<QString, int> i(Data::charlgp_listPos);

		int nb_bones = model_preview ? model_preview->nb_bones() : 0;

		while(i.hasNext()) {
			QCoreApplication::processEvents();
			i.next();
			file = i.key();
			if(file.endsWith(".a", Qt::CaseInsensitive))
			{
				if(nb_bones == 0 || Data::charlgp_animBoneCount.value(file.toLower()) == nb_bones)
					files.append(file.left(file.size()-2).toUpper());
			}
		}

		qSort(files);
		list.addItems(files);

		if(item!=NULL) {
			QString texte = item->text(0).toLower();
			int index;
			if((index=texte.lastIndexOf('.')) != -1)
				texte.truncate(index);
			if((index=list.findText(texte)) != -1)
				list.setCurrentIndex(index);
		}

		QPushButton OKButton(tr("OK"), &dialog);

		QVBoxLayout layout(&dialog);
		layout.addWidget(&list);
		layout.addWidget(&OKButton);

		connect(&list, SIGNAL(currentIndexChanged(QString)), SLOT(modifyAnimation(QString)));
		connect(&OKButton, SIGNAL(released()), &dialog, SLOT(accept()));
		toolBar2->setEnabled(true);

		if(dialog.exec()==QDialog::Accepted) {
			a = list.currentText();
		}
		else {
			if(model_preview) {
				if(item!=NULL)
					model_preview->load(models->currentItem()->text(0), item->text(0), true);
				else
					model_preview->clear();
			}
			return;
		}
	}

	newItem = new QTreeWidgetItem(QStringList(a));
	int page = models->indexOfTopLevelItem(models->currentItem());
	QStringList stringList = field_model_anims.at(page);

	if(item==NULL) {
		model_anims->addTopLevelItem(newItem);
		stringList.append(QString());
	}
	else {
		int row = model_anims->indexOfTopLevelItem(item);
		model_anims->insertTopLevelItem(row+1, newItem);

		stringList.insert(row+1, QString());
	}
	field_model_anims.replace(page, stringList);
	newItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsEditable);
	model_anims->setCurrentItem(newItem);
	if(Data::charlgp_listPos.isEmpty())
		model_anims->editItem(newItem);
}

void ModelManager::modifyAnimation(const QString &a)
{
	if(model_preview)	model_preview->load(models->currentItem()->text(0), a, true);
}

void ModelManager::del_anim()
{
	QTreeWidgetItem *item = model_anims->currentItem();
	if(item==NULL) return;

	int page = models->indexOfTopLevelItem(models->currentItem());
	int row = model_anims->indexOfTopLevelItem(item);
	QStringList stringList = field_model_anims.at(page);
	stringList.removeAt(row);
	field_model_anims.replace(page, stringList);
	if(model_preview)	model_preview->clear();

	delete item;
}

void ModelManager::up_anim()
{
	QTreeWidgetItem *item = model_anims->currentItem();
	if(item==NULL) return;
	int row = model_anims->indexOfTopLevelItem(item);
	if(row<=0)	return;

	QTreeWidgetItem *itemAbove = model_anims->itemAbove(item);

	QString savText = itemAbove->text(0);
	itemAbove->setText(0, item->text(0));
	item->setText(0, savText);

	model_anims->setCurrentItem(itemAbove);
}

void ModelManager::down_anim()
{
	QTreeWidgetItem *item = model_anims->currentItem();
	if(item==NULL) return;

	int row = model_anims->indexOfTopLevelItem(item);
	if(row>=model_anims->topLevelItemCount()-1)	return;
	QTreeWidgetItem *itemBelow = model_anims->itemBelow(item);

	QString savText = itemBelow->text(0);
	itemBelow->setText(0, item->text(0));
	item->setText(0, savText);

	model_anims->setCurrentItem(itemBelow);
}

void ModelManager::renameOK_anim(QTreeWidgetItem *item)
{
	QString text = item->text(0).left(256);
	item->setText(0, text);
	int page = models->indexOfTopLevelItem(models->currentItem());
	int row = model_anims->indexOfTopLevelItem(item);

	QStringList stringList = field_model_anims.at(page);
	stringList.replace(row, text);
	field_model_anims.replace(page, stringList);
}

void ModelManager::show_model(QTreeWidgetItem *item)
{
	if(model_preview) {
		if(item==NULL) {
			model_preview->clear();
			return;
		}
//		qDebug() << "show_model()" << item->text(0);
		model_preview->load(models->currentItem()->text(0), item->text(0), true);
	}
}

void ModelManager::save_page(QTreeWidgetItem *item)
{
	if(model_preview)	model_preview->clear();
	if(!page_filled || item==NULL) return;

	int row = models->indexOfTopLevelItem(item);
	field_model_nameChar.replace(row, model_name->text());
	field_model_unknown.replace(row, model_unknown->value());
	field_model_typeHRC.replace(row, model_typeHRC->value());
	field_colors.replace(row, model_colorDisplay->getColors());
}

void ModelManager::accept()
{
	save_page(models->currentItem());
	fieldModelLoader->typeHRC = typeHRC->value();
	fieldModelLoader->model_unknown = field_model_unknown;
	fieldModelLoader->model_nameChar = field_model_nameChar;
	fieldModelLoader->model_nameHRC = field_model_nameHRC;
	fieldModelLoader->model_typeHRC = field_model_typeHRC;
	fieldModelLoader->model_anims = field_model_anims;
	fieldModelLoader->colors = field_colors;
	fieldModelLoader->setModified(true);
	QDialog::accept();
}

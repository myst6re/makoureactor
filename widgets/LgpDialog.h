#ifndef LGPDIALOG_H
#define LGPDIALOG_H

#include <QtGui>
#include "core/Lgp.h"

class LgpItemModel : public QAbstractItemModel
{
	Q_OBJECT
public:
	LgpItemModel(Lgp *lgp, QObject *parent=0);
	QModelIndex index(int row, int column, const QModelIndex &parent=QModelIndex()) const;
	QModelIndex parent(const QModelIndex &index) const;
	int rowCount(const QModelIndex &parent=QModelIndex()) const;
	int columnCount(const QModelIndex &parent=QModelIndex()) const;
	QVariant data(const QModelIndex &index, int role=Qt::DisplayRole) const;
	QVariant headerData(int section, Qt::Orientation orientation, int role=Qt::DisplayRole) const;
private:
	Lgp *lgp;
	QStringList fileList;
	QIcon fileIcon;
};

class LgpDialog : public QDialog, ArchiveObserver
{
	Q_OBJECT
public:
	LgpDialog(Lgp *lgp, QWidget *parent=0);

	bool observerWasCanceled() const;
	void setObserverMaximum(unsigned int max);
	void setObserverValue(int value);
signals:
	void modified();
private slots:
	void renameCurrent();
	void replaceCurrent();
	void extractCurrent();
	void setButtonsState();
	void pack();
private:
	Lgp *lgp;
	QTreeView *treeView;
	QPushButton *extractButton, *renameButton, *replaceButton, *packButton;
	QProgressDialog *progressDialog;
};

#endif // LGPDIALOG_H

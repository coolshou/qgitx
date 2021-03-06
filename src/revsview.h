/*
	Author: Marco Costalba (C) 2005-2007

	Copyright: See COPYING file that comes with this distribution

*/
#ifndef REVSVIEW_H
#define REVSVIEW_H

#include <QPointer>
#include "ui_revsview.h" // needed by moc_* file to understand tab() function
#include "common.h"
#include "domain.h"

class MainImpl;
class Git;
class FileHistory;
class PatchView;

class RevsView : public Domain {
Q_OBJECT
public:
	RevsView(MainImpl* parent, Git* git, bool isMain = false);
	~RevsView();
	void clear(bool complete);
	void setEnabled(bool b);
	Ui_TabRev* tab() { return revTab; }

private slots:
	void on_newRevsAdded(const FileHistory*, const QVector<ShaString>&);
	void on_loadCompleted(const FileHistory*, const QString& stats);
	void on_lanesContextMenuRequested(const QStringList&, const QStringList&);
	void on_updateRevDesc();

protected:
	virtual bool doUpdate(bool force);

private:
	friend class MainImpl;

	void updateLineEditSHA(bool clear = false);

	Ui_TabRev* revTab;
};

#endif

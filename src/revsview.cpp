/*
	Description: qgit revision list view

	Author: Marco Costalba (C) 2005-2007

	Copyright: See COPYING file that comes with this distribution

*/
#include <QMenu>
#include <QStackedWidget>
#include "common.h"
#include "git.h"
#include "domain.h"
#include "historyview.h"
#include "revdesc.h"
#include "smartbrowse.h"
#include "mainimpl.h"
#include "revsview.h"

RevsView::RevsView(MainImpl* mi, Git* g, bool isMain) : Domain(mi, g, isMain) {

	revTab = new Ui_TabRev();
	revTab->setupUi(container);

	tab()->listViewLog->setup(this, g);
	tab()->textBrowserDesc->setup(this);
	tab()->textEditDiff->setup(this, git);

	setTabLogDiffVisible(QGit::testFlag(QGit::LOG_DIFF_TAB_F));

	SmartBrowse* sb = new SmartBrowse(this);

	// restore geometry
	QVector<QSplitter*> v;
	v << tab()->horizontalSplitter << tab()->verticalSplitter;
	QGit::restoreGeometrySetting(QGit::REV_GEOM_KEY, NULL, &v);

	connect(m(), SIGNAL(typeWriterFontChanged()),
	        tab()->textEditDiff, SLOT(typeWriterFontChanged()));

	connect(m(), SIGNAL(flagChanged(uint)),
	        sb, SLOT(flagChanged(uint)));

	connect(git, SIGNAL(newRevsAdded(const FileHistory*, const QVector<ShaString>&)),
	        this, SLOT(on_newRevsAdded(const FileHistory*, const QVector<ShaString>&)));

	connect(git, SIGNAL(loadCompleted(const FileHistory*, const QString&)),
	        this, SLOT(on_loadCompleted(const FileHistory*, const QString&)));

	connect(m(), SIGNAL(changeFont(const QFont&)),
	        tab()->listViewLog, SLOT(on_changeFont(const QFont&)));

	connect(m(), SIGNAL(updateRevDesc()), this, SLOT(on_updateRevDesc()));

	connect(tab()->listViewLog, SIGNAL(lanesContextMenuRequested(const QStringList&,
	        const QStringList&)), this, SLOT(on_lanesContextMenuRequested
	       (const QStringList&, const QStringList&)));

	connect(tab()->listViewLog, SIGNAL(revisionsDragged(const QStringList&)),
	        m(), SLOT(revisionsDragged(const QStringList&)));

	connect(tab()->listViewLog, SIGNAL(revisionsDropped(const QStringList&)),
	        m(), SLOT(revisionsDropped(const QStringList&)));

	connect(tab()->listViewLog, SIGNAL(contextMenu(const QString&, int)),
	        this, SLOT(on_contextMenu(const QString&, int)));

    //FIXME: remove me
    /*connect(m()->treeView, SIGNAL(contextMenu(const QString&, int)),
            this, SLOT(on_contextMenu(const QString&, int)));*/

	connect(m(), SIGNAL(highlightPatch(const QString&, bool)),
	        tab()->textEditDiff, SLOT(on_highlightPatch(const QString&, bool)));
}

RevsView::~RevsView() {

	if (!parent())
		return;

	QVector<QSplitter*> v;
	v << tab()->horizontalSplitter << tab()->verticalSplitter;
	QGit::saveGeometrySetting(QGit::REV_GEOM_KEY, NULL, &v);

	// manually delete before container is removed in Domain
	// d'tor to avoid a crash due to spurious events in
	// SmartBrowse::eventFilter()
	delete tab()->textBrowserDesc;
	delete tab()->textEditDiff;

	delete revTab;
}

void RevsView::clear(bool complete) {

	Domain::clear(complete);

	tab()->textBrowserDesc->clear();
	tab()->textEditDiff->clear();
}

void RevsView::setEnabled(bool b) {

	container->setEnabled(b);
}

void RevsView::toggleDiffView() {

	QStackedWidget* s = tab()->stackedPanes;
	QTabWidget* t = tab()->tabLogDiff;

	bool isTabPage = (s->currentIndex() == 0);
	int idx = (isTabPage ? t->currentIndex() : s->currentIndex());

	bool old = container->updatesEnabled();
	container->setUpdatesEnabled(false);

	if (isTabPage)
		t->setCurrentIndex(1 - idx);
	else
		s->setCurrentIndex(3 - idx);

	container->setUpdatesEnabled(old);
}

void RevsView::setTabLogDiffVisible(bool b) {

	QStackedWidget* s = tab()->stackedPanes;
	QTabWidget* t = tab()->tabLogDiff;

	bool isTabPage = (s->currentIndex() == 0);
	int idx = (isTabPage ? t->currentIndex() : s->currentIndex());

	container->setUpdatesEnabled(false);

	if (b && !isTabPage) {

		t->addTab(tab()->textBrowserDesc, "Log");
		t->addTab(tab()->textEditDiff, "Diff");

		t->setCurrentIndex(idx - 1);
		s->setCurrentIndex(0);
	}
	if (!b && isTabPage) {

		s->addWidget(tab()->textBrowserDesc);
		s->addWidget(tab()->textEditDiff);

		// manually remove the two remaining empty pages
		t->removeTab(0); t->removeTab(0);

		s->setCurrentIndex(idx + 1);
	}
	container->setUpdatesEnabled(true);
}

void RevsView::on_newRevsAdded(const FileHistory* fh, const QVector<ShaString>&) {

	if (!git->isMainHistory(fh) || !st.sha().isEmpty())
		return;

    HistoryView* lv = tab()->listViewLog;
	if (lv->model()->rowCount() == 0)
		return;

	st.setSha(lv->sha(0));
	st.setSelectItem(true);
	UPDATE(); // give feedback to user as soon as possible
}

void RevsView::on_loadCompleted(const FileHistory* fh, const QString& stats) {

	if (!git->isMainHistory(fh))
		return;

	UPDATE(); // restore revision after a refresh
	QApplication::postEvent(this, new MessageEvent(stats));
}

void RevsView::on_updateRevDesc() {

	SCRef d = m()->getRevisionDesc(st.sha());
	tab()->textBrowserDesc->setHtml(d);
}

bool RevsView::doUpdate(bool force) {

	bool found = tab()->listViewLog->update();

	if (!found && !st.sha().isEmpty()) {

		const QString tmp("Sorry, revision " + st.sha() +
		                  " has not been found in main view");
		showStatusBarMessage(tmp);

	} else { // sha could be NULL

		if (st.isChanged(StateInfo::SHA) || force) {
			on_updateRevDesc();
			showStatusBarMessage(git->getRevInfo(st.sha()));

			if (   testFlag(QGit::MSG_ON_NEW_F)
			    && tab()->textEditDiff->isVisible())
				toggleDiffView();
		}
		const RevFile* files = NULL;
		bool newFiles = false;

		if (st.isChanged(StateInfo::ANY & ~StateInfo::FILE_NAME) || force) {
			// blocking call, could be slow in case of all merge files
			files = git->getFiles(st.sha(), st.diffToSha(), st.allMergeFiles());
			newFiles = true;

			tab()->textEditDiff->update(st);
		}

		if (st.selectItem()) {
            bool isDir = st.isDir();
			m()->updateContextActions(st.sha(), st.fileName(), isDir, found);
		}
		if (st.isChanged() || force)
			tab()->textEditDiff->centerOnFileHeader(st);
	}
	return (found || st.sha().isEmpty());
}

void RevsView::on_lanesContextMenuRequested(SCList parents, SCList childs) {

	QMenu contextMenu;
	FOREACH_SL (it, childs)
		contextMenu.addAction("Child: " + git->getShortLog(*it));

	FOREACH_SL (it, parents) {
		QString log(git->getShortLog(*it));
		contextMenu.addAction("Parent: " + (log.isEmpty() ? *it : log));
	}
	QAction* act = contextMenu.exec(QCursor::pos()); // modal exec
	if (!act)
		return;

	int cc = childs.count();
	int id = contextMenu.actions().indexOf(act);
	SCRef target(id < cc ? childs[id] : parents[id - cc]);
	st.setSha(target);
	UPDATE();
}

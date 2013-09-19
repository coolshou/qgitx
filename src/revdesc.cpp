/*
	Author: Marco Costalba (C) 2005-2007

	Copyright: See COPYING file that comes with this distribution
*/
#include <QApplication>
#include <QMenu>
#include <QContextMenuEvent>
#include <QRegExp>
#include <QClipboard>
#include <QWebFrame>
#include <QRegularExpression>
#include "domain.h"
#include "revdesc.h"

RevDesc::RevDesc(QWidget* p) : QWebView(p), d(NULL) {

    //FIXME signals are not available in QWebView
/* 	connect(this, SIGNAL(anchorClicked(const QUrl&)),
	        this, SLOT(on_anchorClicked(const QUrl&)));

	connect(this, SIGNAL(highlighted(const QUrl&)),
            this, SLOT(on_highlighted(const QUrl&)));*/
    this->page()->setLinkDelegationPolicy(QWebPage::DelegateAllLinks);
    connect(this, &QWebView::linkClicked, this, &RevDesc::on_anchorClicked);
}

void RevDesc::on_anchorClicked(const QUrl& link) {
    static QRegularExpression anchorRE("^#(.+)$");
    qDebug() << "clicked on " << link.toDisplayString() << "\n";
    QWebFrame* frame = this->page()->mainFrame();

    QRegularExpressionMatch anchorMatch = anchorRE.match(link.toDisplayString());
    if(anchorMatch.hasMatch()) {
        frame->scrollToAnchor(anchorMatch.captured(1));
    }
}

void RevDesc::on_highlighted(const QUrl& link) {

	highlightedLink = link.toString();
}

void RevDesc::on_linkCopy() {

	QClipboard* cb = QApplication::clipboard();
	cb->setText(highlightedLink);
}

void RevDesc::contextMenuEvent(QContextMenuEvent* e) {

    //FIXME as this is no longer a QTextEdit widget, we no longer have createStandardContextMenu
/*	QMenu* menu = createStandardContextMenu();
	if (!highlightedLink.isEmpty()) {
		QAction* act = menu->addAction("Copy link SHA1");
		connect(act, SIGNAL(triggered()), this, SLOT(on_linkCopy()));
	}
	menu->exec(e->globalPos());
    delete menu;*/
}

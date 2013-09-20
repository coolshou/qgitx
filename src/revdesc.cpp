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

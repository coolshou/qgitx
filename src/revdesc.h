/*
	Author: Marco Costalba (C) 2005-2007

	Copyright: See COPYING file that comes with this distribution
*/
#ifndef REVDESC_H
#define REVDESC_H

#include <QWebView>

class Domain;

class RevDesc: public QWebView {
Q_OBJECT
public:
	RevDesc(QWidget* parent);
	void setup(Domain* dm) { d = dm; }

private slots:
    void on_anchorClicked(const QUrl& link);

private:
	Domain* d;
	QString highlightedLink;
};

#endif

/*
	Description: qgit revision list view

	Author: Marco Costalba (C) 2005-2007

	Copyright: See COPYING file that comes with this distribution

*/
#include <QApplication>
#include <QHeaderView>
#include <QMouseEvent>
#include <QPainter>
#include <QPixmap>
#include <QShortcut>
#include <QMimeData>
#include "domain.h"
#include "git.h"
#include "historyview.h"
#include "filehistory.h"

using namespace QGit;

HistoryView::HistoryView(QWidget* parent) : QTreeView(parent), d(NULL), git(NULL), fh(NULL), lp(NULL) {}

void HistoryView::setup(Domain* dm, Git* g) {

	d = dm;
	git = g;
	fh = d->model();
	st = &(d->st);
	filterNextContextMenuRequest = false;

	setFont(QGit::STD_FONT);

	// create ListViewProxy unplugged, will be plug
	// to the model only when filtering is needed
	lp = new ListViewProxy(this, d, git);
	setModel(fh);

	ListViewDelegate* lvd = new ListViewDelegate(git, lp, this);
	lvd->setLaneHeight(fontMetrics().height());
	setItemDelegate(lvd);

	setupGeometry(); // after setting delegate

	// shortcuts are activated only if widget is visible, this is good
	new QShortcut(Qt::Key_Up,   this, SLOT(on_keyUp()));
	new QShortcut(Qt::Key_Down, this, SLOT(on_keyDown()));

	connect(lvd, SIGNAL(updateView()), viewport(), SLOT(update()));

	connect(this, SIGNAL(diffTargetChanged(int)), lvd, SLOT(diffTargetChanged(int)));

	connect(this, SIGNAL(customContextMenuRequested(const QPoint&)),
	        this, SLOT(on_customContextMenuRequested(const QPoint&)));
}

HistoryView::~HistoryView() {

	git->cancelDataLoading(fh); // non blocking
}

const QString HistoryView::sha(int row) const {

	if (!lp->sourceModel()) // unplugged
		return fh->sha(row);

	QModelIndex idx = lp->mapToSource(lp->index(row, 0));
	return fh->sha(idx.row());
}

int HistoryView::row(SCRef sha) const {

	if (!lp->sourceModel()) // unplugged
		return fh->row(sha);

	int row = fh->row(sha);
	QModelIndex idx = fh->index(row, 0);
	return lp->mapFromSource(idx).row();
}

void HistoryView::setupGeometry() {

	QPalette pl = palette();
	pl.setColor(QPalette::Base, ODD_LINE_COL);
	pl.setColor(QPalette::AlternateBase, EVEN_LINE_COL);
	setPalette(pl); // does not seem to inherit application paletteAnnotate

	QHeaderView* hv = header();
	hv->setStretchLastSection(true);
    hv->setSectionResizeMode(LOG_COL, QHeaderView::Interactive);
    hv->setSectionResizeMode(TIME_COL, QHeaderView::Interactive);
    hv->setSectionResizeMode(ANN_ID_COL, QHeaderView::ResizeToContents);
	hv->resizeSection(GRAPH_COL, DEF_GRAPH_COL_WIDTH);
	hv->resizeSection(LOG_COL, DEF_LOG_COL_WIDTH);
	hv->resizeSection(AUTH_COL, DEF_AUTH_COL_WIDTH);
	hv->resizeSection(TIME_COL, DEF_TIME_COL_WIDTH);

	if (git->isMainHistory(fh))
		hideColumn(ANN_ID_COL);
}

void HistoryView::scrollToNextHighlighted(int direction) {

	// Depending on the value of direction, scroll to:
	// -1 = the next highlighted item above the current one (i.e. newer in history)
	//  1 = the next highlighted item below the current one (i.e. older in history)
	//  0 = the first highlighted item from the top of the list

	QModelIndex idx = currentIndex();

	if (!direction) {
		idx = idx.sibling(0,0);
		if (lp->isHighlighted(idx.row())) {
			setCurrentIndex(idx);
			return;
		}
	}

	do {
		idx = (direction >= 0 ? indexBelow(idx) : indexAbove(idx));
		if (!idx.isValid())
			return;

	} while (!lp->isHighlighted(idx.row()));

	setCurrentIndex(idx);
}

void HistoryView::scrollToCurrent(ScrollHint hint) {

	if (currentIndex().isValid())
		scrollTo(currentIndex(), hint);
}

void HistoryView::on_keyUp() {

	QModelIndex idx = indexAbove(currentIndex());
	if (idx.isValid())
		setCurrentIndex(idx);
}

void HistoryView::on_keyDown() {

	QModelIndex idx = indexBelow(currentIndex());
	if (idx.isValid())
		setCurrentIndex(idx);
}

void HistoryView::on_changeFont(const QFont& f) {

	setFont(f);
	ListViewDelegate* lvd = static_cast<ListViewDelegate*>(itemDelegate());
	lvd->setLaneHeight(fontMetrics().height());
	scrollToCurrent();
}

const QString HistoryView::currentText(int column) {

	QModelIndex idx = model()->index(currentIndex().row(), column);
	return (idx.isValid() ? idx.data().toString() : "");
}

int HistoryView::getLaneType(SCRef sha, int pos) const {

	const Rev* r = git->revLookup(sha, fh);
	return (r && pos < r->lanes.count() && pos >= 0 ? r->lanes.at(pos) : -1);
}

void HistoryView::showIdValues() {

	fh->setAnnIdValid();
	viewport()->update();
}

void HistoryView::getSelectedItems(QStringList& selectedItems) {

	selectedItems.clear();
	QModelIndexList ml = selectionModel()->selectedRows();
	FOREACH (QModelIndexList, it, ml)
		selectedItems.append(sha((*it).row()));

	// selectedRows() returns the items in an unspecified order,
	// so be sure rows are ordered from newest to oldest.
	selectedItems = git->sortShaListByIndex(selectedItems);
}

const QString HistoryView::shaFromAnnId(uint id) {

	if (git->isMainHistory(fh))
		return "";

	return sha(model()->rowCount() - id);
}

int HistoryView::filterRows(bool isOn, bool highlight, SCRef filter, int colNum, ShaSet* set) {

	setUpdatesEnabled(false);
	int matchedNum = lp->setFilter(isOn, highlight, filter, colNum, set);
	viewport()->update();
	setUpdatesEnabled(true);
	UPDATE_DOMAIN(d);
	return matchedNum;
}

bool HistoryView::update() {

	int stRow = row(st->sha());
	if (stRow == -1)
		return false; // main/tree view asked us a sha not in history

	QModelIndex index = currentIndex();
	QItemSelectionModel* sel = selectionModel();

	if (index.isValid() && (index.row() == stRow)) {

		if (sel->isSelected(index) != st->selectItem())
			sel->select(index, QItemSelectionModel::Toggle);

		scrollTo(index);
	} else {
		// setCurrentIndex() does not clear previous
		// selections in a multi selection QListView
		clearSelection();

		QModelIndex newIndex = model()->index(stRow, 0);
		if (newIndex.isValid()) {

			// emits QItemSelectionModel::currentChanged()
			setCurrentIndex(newIndex);
			scrollTo(newIndex);
			if (!st->selectItem())
				sel->select(newIndex, QItemSelectionModel::Deselect);
		}
	}
	if (git->isMainHistory(fh))
		emit diffTargetChanged(row(st->diffToSha()));

	return currentIndex().isValid();
}

void HistoryView::currentChanged(const QModelIndex& index, const QModelIndex&) {

	SCRef selRev = sha(index.row());
	if (st->sha() != selRev) { // to avoid looping
		st->setSha(selRev);
		st->setSelectItem(true);
		UPDATE_DOMAIN(d);
	}
}

bool HistoryView::filterRightButtonPressed(QMouseEvent* e) {

	QModelIndex index = indexAt(e->pos());
	SCRef selSha = sha(index.row());
	if (selSha.isEmpty())
		return false;

	if (e->modifiers() == Qt::ControlModifier) { // check for 'diff to' function

		if (selSha != ZERO_SHA && st->sha() != ZERO_SHA) {

			if (selSha != st->diffToSha())
				st->setDiffToSha(selSha);
			else
				st->setDiffToSha(""); // restore std view

			filterNextContextMenuRequest = true;
			UPDATE_DOMAIN(d);
			return true; // filter event out
		}
	}
	// check for 'children & parents' function, i.e. if mouse is on the graph
	if (index.column() == GRAPH_COL) {

		filterNextContextMenuRequest = true;
		QStringList parents, children;
		if (getLaneParentsChilds(selSha, e->pos().x(), parents, children))
			emit lanesContextMenuRequested(parents, children);

		return true; // filter event out
	}
	return false;
}

void HistoryView::mousePressEvent(QMouseEvent* e) {

	if (currentIndex().isValid() && e->button() == Qt::LeftButton)
		d->setReadyToDrag(true);

	if (e->button() == Qt::RightButton && filterRightButtonPressed(e))
		return; // filtered out

	QTreeView::mousePressEvent(e);
}

void HistoryView::mouseReleaseEvent(QMouseEvent* e) {

	d->setReadyToDrag(false); // in case of just click without moving
	QTreeView::mouseReleaseEvent(e);
}

void HistoryView::mouseMoveEvent(QMouseEvent* e) {

	if (d->isReadyToDrag()) {

		if (indexAt(e->pos()).row() == currentIndex().row())
			return; // move at least by one line to activate drag

		if (!d->setDragging(true))
			return;

		QStringList selRevs;
		getSelectedItems(selRevs);
		selRevs.removeAll(ZERO_SHA);
		if (!selRevs.empty())
			emit revisionsDragged(selRevs); // blocking until drop event

		d->setDragging(false);
	}
	QTreeView::mouseMoveEvent(e);
}

void HistoryView::dragEnterEvent(QDragEnterEvent* e) {

	if (e->mimeData()->hasFormat("text/plain"))
		e->accept();
}

void HistoryView::dragMoveEvent(QDragMoveEvent* e) {

	// already checked by dragEnterEvent()
	e->accept();
}

void HistoryView::dropEvent(QDropEvent *e) {

	SCList remoteRevs(e->mimeData()->text().split('\n', QString::SkipEmptyParts));
	if (!remoteRevs.isEmpty()) {
		// some sanity check on dropped data
		SCRef sha(remoteRevs.first().section('@', 0, 0));
		SCRef remoteRepo(remoteRevs.first().section('@', 1));
		if (sha.length() == 40 && !remoteRepo.isEmpty())
			emit revisionsDropped(remoteRevs);
	}
}

void HistoryView::on_customContextMenuRequested(const QPoint& pos) {

	QModelIndex index = indexAt(pos);
	if (!index.isValid())
		return;

	if (filterNextContextMenuRequest) {
		// event filter does not work on them
		filterNextContextMenuRequest = false;
		return;
	}
	emit contextMenu(sha(index.row()), POPUP_LIST_EV);
}

bool HistoryView::getLaneParentsChilds(SCRef sha, int x, SList p, SList c) {

	ListViewDelegate* lvd = static_cast<ListViewDelegate*>(itemDelegate());
	uint lane = x / lvd->laneWidth();
	int t = getLaneType(sha, lane);
	if (t == EMPTY || t == -1)
		return false;

	// first find the parents
	p.clear();
	QString root;
	if (!isFreeLane(t)) {
		p = git->revLookup(sha, fh)->parents(); // pointer cannot be NULL
		root = sha;
	} else {
		SCRef par(git->getLaneParent(sha, lane));
		if (par.isEmpty()) {
			dbs("ASSERT getLaneParentsChilds: parent not found");
			return false;
		}
		p.append(par);
		root = p.first();
	}
	// then find children
	c = git->getChilds(root);
	return true;
}

// *****************************************************************************

ListViewDelegate::ListViewDelegate(Git* g, ListViewProxy* px, QObject* p) : QItemDelegate(p) {

	git = g;
	lp = px;
	laneHeight = 0;
	diffTargetRow = -1;
}

QSize ListViewDelegate::sizeHint(const QStyleOptionViewItem&, const QModelIndex&) const {

	return QSize(laneWidth(), laneHeight);
}

void ListViewDelegate::diffTargetChanged(int row) {

	if (diffTargetRow != row) {
		diffTargetRow = row;
		emit updateView();
	}
}

const Rev* ListViewDelegate::revLookup(int row, FileHistory** fhPtr) const {

    HistoryView* lv = static_cast<HistoryView*>(parent());
	FileHistory* fh = static_cast<FileHistory*>(lv->model());

	if (lp->sourceModel())
		fh = static_cast<FileHistory*>(lp->sourceModel());

	if (fhPtr)
		*fhPtr = fh;

	return git->revLookup(lv->sha(row), fh);
}

static QColor blend(const QColor& col1, const QColor& col2, int amount = 128) {

	// Returns ((256 - amount)*col1 + amount*col2) / 256;
	return QColor(((256 - amount)*col1.red()   + amount*col2.red()  ) / 256,
	              ((256 - amount)*col1.green() + amount*col2.green()) / 256,
	              ((256 - amount)*col1.blue()  + amount*col2.blue() ) / 256);
}

void ListViewDelegate::paintGraphLane(QPainter* p, int type, int x1, int x2,
                                      const QColor& col, const QColor& activeCol, const QBrush& back) const {

	int h = laneHeight / 2;
	int m = (x1 + x2) / 2;
	int r = (x2 - x1) / 3;
	int d =  2 * r;

	#define P_CENTER m , h
	#define P_0      x2, h      // >
	#define P_90     m , 0      // ^
	#define P_180    x1, h      // <
	#define P_270    m , 2 * h  // v
	#define DELTA_UR 2*(x1 - m), 2*h ,   0*16, 90*16  // -,
	#define DELTA_DR 2*(x1 - m), 2*-h, 270*16, 90*16  // -'
	#define DELTA_UL 2*(x2 - m), 2*h ,  90*16, 90*16  //  ,-
	#define DELTA_DL 2*(x2 - m), 2*-h, 180*16, 90*16  //  '-
	#define CENTER_UR x1, 2*h, 225
	#define CENTER_DR x1, 0  , 135
	#define CENTER_UL x2, 2*h, 315
	#define CENTER_DL x2, 0  ,  45
	#define R_CENTER m - r, h - r, d, d

	static QPen myPen(Qt::black, 2); // fast path here
    static QPen blackSolidThickPen(Qt::black, 2, Qt::SolidLine);
    static QBrush whiteBrush(Qt::white);

	// arc
	switch (type) {
	case JOIN:
	case JOIN_R:
	case HEAD:
	case HEAD_R: {
		QConicalGradient gradient(CENTER_UR);
		gradient.setColorAt(0.375, col);
		gradient.setColorAt(0.625, activeCol);
		myPen.setBrush(gradient);
		p->setPen(myPen);
		p->drawArc(P_CENTER, DELTA_UR);
		break;
	}
	case JOIN_L: {
		QConicalGradient gradient(CENTER_UL);
		gradient.setColorAt(0.375, activeCol);
		gradient.setColorAt(0.625, col);
		myPen.setBrush(gradient);
		p->setPen(myPen);
		p->drawArc(P_CENTER, DELTA_UL);
		break;
	}
	case TAIL:
	case TAIL_R: {
		QConicalGradient gradient(CENTER_DR);
		gradient.setColorAt(0.375, activeCol);
		gradient.setColorAt(0.625, col);
		myPen.setBrush(gradient);
		p->setPen(myPen);
		p->drawArc(P_CENTER, DELTA_DR);
		break;
	}
	default:
		break;
	}

	myPen.setColor(col);
	p->setPen(myPen);

	// vertical line
	switch (type) {
	case ACTIVE:
	case NOT_ACTIVE:
	case MERGE_FORK:
	case MERGE_FORK_R:
	case MERGE_FORK_L:
	case JOIN:
	case JOIN_R:
	case JOIN_L:
	case CROSS:
		p->drawLine(P_90, P_270);
		break;
	case HEAD_L:
	case BRANCH:
		p->drawLine(P_CENTER, P_270);
		break;
	case TAIL_L:
	case INITIAL:
	case BOUNDARY:
	case BOUNDARY_C:
	case BOUNDARY_R:
	case BOUNDARY_L:
		p->drawLine(P_90, P_CENTER);
		break;
	default:
		break;
	}

	myPen.setColor(activeCol);
	p->setPen(myPen);

	// horizontal line
	switch (type) {
	case MERGE_FORK:
	case JOIN:
	case HEAD:
	case TAIL:
	case CROSS:
	case CROSS_EMPTY:
	case BOUNDARY_C:
		p->drawLine(P_180, P_0);
		break;
	case MERGE_FORK_R:
	case BOUNDARY_R:
		p->drawLine(P_180, P_CENTER);
		break;
	case MERGE_FORK_L:
	case HEAD_L:
	case TAIL_L:
	case BOUNDARY_L:
		p->drawLine(P_CENTER, P_0);
		break;
	default:
		break;
	}

	// center symbol, e.g. rect or ellipse
	switch (type) {
	case ACTIVE:
	case INITIAL:
	case BRANCH:
        p->setPen(blackSolidThickPen);
        p->setBrush(whiteBrush);
		p->drawEllipse(R_CENTER);
        p->setPen(Qt::NoPen);
		break;
	case MERGE_FORK:
	case MERGE_FORK_R:
	case MERGE_FORK_L:
        p->setPen(blackSolidThickPen);
        p->setBrush(whiteBrush);
        p->drawEllipse(R_CENTER);
        p->setPen(Qt::NoPen);
		break;
	case UNAPPLIED:
		// Red minus sign
		p->setPen(Qt::NoPen);
		p->setBrush(Qt::red);
		p->drawRect(m - r, h - 1, d, 2);
		break;
	case APPLIED:
		// Green plus sign
		p->setPen(Qt::NoPen);
		p->setBrush(DARK_GREEN);
		p->drawRect(m - r, h - 1, d, 2);
		p->drawRect(m - 1, h - r, 2, d);
		break;
	case BOUNDARY:
		p->setBrush(back);
		p->drawEllipse(R_CENTER);
		break;
	case BOUNDARY_C:
	case BOUNDARY_R:
	case BOUNDARY_L:
		p->setBrush(back);
		p->drawRect(R_CENTER);
		break;
	default:
		break;
	}
	#undef P_CENTER
	#undef P_0
	#undef P_90
	#undef P_180
	#undef P_270
	#undef DELTA_UR
	#undef DELTA_DR
	#undef DELTA_UL
	#undef DELTA_DL
	#undef CENTER_UR
	#undef CENTER_DR
	#undef CENTER_UL
	#undef CENTER_DL
	#undef R_CENTER
}

void ListViewDelegate::paintGraph(QPainter* p, const QStyleOptionViewItem& opt,
                                  const QModelIndex& i) const {
    static const QColor colors[COLORS_NUM] = { SOLAR_YELLOW, SOLAR_ORANGE, SOLAR_RED,
                                               SOLAR_MAGENTA, SOLAR_VIOLET, SOLAR_BLUE,
                                               SOLAR_CYAN, SOLAR_GREEN };
	if (opt.state & QStyle::State_Selected)
		p->fillRect(opt.rect, opt.palette.highlight());
	else if (i.row() & 1)
		p->fillRect(opt.rect, opt.palette.alternateBase());
	else
		p->fillRect(opt.rect, opt.palette.base());

	FileHistory* fh;
	const Rev* r = revLookup(i.row(), &fh);
	if (!r)
		return;

	p->save();
	p->setClipRect(opt.rect, Qt::IntersectClip);
	p->translate(opt.rect.topLeft());

	// calculate lanes
	if (r->lanes.count() == 0)
		git->setLane(r->sha(), fh);

	QBrush back = opt.palette.base();
	const QVector<int>& lanes(r->lanes);
	uint laneNum = lanes.count();
	uint activeLane = 0;
	for (uint i = 0; i < laneNum; i++)
		if (isActive(lanes[i])) {
			activeLane = i;
			break;
		}

	int x1 = 0, x2 = 0;
	int maxWidth = opt.rect.width();
	int lw = laneWidth();
	QColor activeColor = colors[activeLane % COLORS_NUM];
	if (opt.state & QStyle::State_Selected)
		activeColor = blend(activeColor, opt.palette.highlightedText().color(), 208);
	for (uint i = 0; i < laneNum && x2 < maxWidth; i++) {

		x1 = x2;
		x2 += lw;

		int ln = lanes[i];
		if (ln == EMPTY)
			continue;

		QColor color = i == activeLane ? activeColor : colors[i % COLORS_NUM];
		paintGraphLane(p, ln, x1, x2, color, activeColor, back);
	}
	p->restore();
}

void ListViewDelegate::paintLog(QPainter* p, const QStyleOptionViewItem& opt,
                                const QModelIndex& index) const {

	int row = index.row();
	const Rev* r = revLookup(row);
	if (!r)
		return;

	if (r->isDiffCache)
		p->fillRect(opt.rect, changedFiles(ZERO_SHA) ? ORANGE : DARK_ORANGE);

	if (diffTargetRow == row)
		p->fillRect(opt.rect, LIGHT_BLUE);

	bool isHighlighted = lp->isHighlighted(row);
	QPixmap* pm = getTagMarks(r->sha(), opt);

	if (!pm && !isHighlighted) { // fast path in common case
		QItemDelegate::paint(p, opt, index);
		return;
	}
	QStyleOptionViewItem newOpt(opt); // we need a copy
	if (pm) {
		p->drawPixmap(newOpt.rect.x(), newOpt.rect.y(), *pm);
		newOpt.rect.adjust(pm->width(), 0, 0, 0);
		delete pm;
	}
	if (isHighlighted)
		newOpt.font.setBold(true);

	QItemDelegate::paint(p, newOpt, index);
}

void ListViewDelegate::paint(QPainter* p, const QStyleOptionViewItem& opt,
                             const QModelIndex& index) const {

  p->setRenderHints(QPainter::Antialiasing);

	if (index.column() == GRAPH_COL)
		return paintGraph(p, opt, index);

	if (index.column() == LOG_COL)
		return paintLog(p, opt, index);

	return QItemDelegate::paint(p, opt, index);
}

bool ListViewDelegate::changedFiles(SCRef sha) const {

	const RevFile* f = git->getFiles(sha);
	if (f)
		for (int i = 0; i < f->count(); i++)
			if (!f->statusCmp(i, RevFile::UNKNOWN))
				return true;
	return false;
}

QPixmap* ListViewDelegate::getTagMarks(SCRef sha, const QStyleOptionViewItem& opt) const {

	uint rt = git->checkRef(sha);
	if (rt == 0)
		return NULL; // common case

	QPixmap* pm = new QPixmap(); // must be deleted by caller

	if (rt & Git::BRANCH)
		addRefPixmap(&pm, sha, Git::BRANCH, opt);

	if (rt & Git::RMT_BRANCH)
		addRefPixmap(&pm, sha, Git::RMT_BRANCH, opt);

	if (rt & Git::TAG)
		addRefPixmap(&pm, sha, Git::TAG, opt);

	if (rt & Git::REF)
		addRefPixmap(&pm, sha, Git::REF, opt);

	return pm;
}

void ListViewDelegate::addRefPixmap(QPixmap** pp, SCRef sha, int type, QStyleOptionViewItem opt) const {

	QString curBranch;
	SCList refs = git->getRefName(sha, (Git::RefType)type, &curBranch);
	FOREACH_SL (it, refs) {

		bool isCur = (curBranch == *it);
		opt.font.setBold(isCur);

		QColor clr;
		if (type == Git::BRANCH)
            clr = (isCur ? QColor(0xfc, 0xa6, 0x4f) : QColor(0xaa, 0xf2, 0x54));

		else if (type == Git::RMT_BRANCH)
            clr = QColor(0xb2, 0xdf, 0xff);

		else if (type == Git::TAG)
            clr = QColor(0xFC, 0xED, 0x4f);

		else if (type == Git::REF)
            clr = Qt::yellow;

		opt.palette.setColor(QPalette::Window, clr);
		addTextPixmap(pp, *it, opt);
	}
}

void ListViewDelegate::addTextPixmap(QPixmap** pp, SCRef txt, const QStyleOptionViewItem& opt) const {

	QPixmap* pm = *pp;
    int ofs = pm->isNull() ? 1 : pm->width() + 3;
	int spacing = 2;
    QFont smallerFont(opt.font);
    smallerFont.setPointSizeF(opt.font.pointSizeF()*0.8f);
    QFontMetrics fm(smallerFont);
    QFontMetrics fmBigFont(opt.font);
    int pw = fm.boundingRect(txt).width() + 2 * (spacing + int(smallerFont.bold()));
    int ph = fm.height();
    int y_offset = (fmBigFont.height() - ph)/2;

    QPixmap* newPm = new QPixmap(ofs + pw + 2, fmBigFont.height());
	QPainter p;
	p.begin(newPm);
    p.setRenderHints(QPainter::Antialiasing);
    newPm->fill(opt.palette.base().color());
	if (!pm->isNull()) {
		p.drawPixmap(0, 0, *pm);
	}
	p.setPen(opt.palette.color(QPalette::WindowText));
	p.setBrush(opt.palette.color(QPalette::Window));
    p.setFont(smallerFont);
    float borderRadius = std::min(pw,ph)/4.0f;
    p.drawRoundedRect(ofs, y_offset, pw, ph, borderRadius, borderRadius);
    p.drawText(ofs + spacing, fm.ascent() + y_offset, txt);
	p.end();

	delete pm;
	*pp = newPm;
}

// *****************************************************************************

ListViewProxy::ListViewProxy(QObject* p, Domain * dm, Git * g) : QSortFilterProxyModel(p) {

	d = dm;
	git = g;
	colNum = 0;
	isHighLight = false;
	setDynamicSortFilter(false);
}

bool ListViewProxy::isMatch(SCRef sha) const {

	if (colNum == SHA_MAP_COL)
		// in this case shaMap contains all good sha to search for
		return shaSet.contains(sha);

	const Rev* r = git->revLookup(sha);
	if (!r) {
		dbp("ASSERT in ListViewFilter::isMatch, sha <%1> not found", sha);
		return false;
	}
	QString target;
	if (colNum == LOG_COL)
		target = r->shortLog();
	else if (colNum == AUTH_COL)
		target = r->author();
	else if (colNum == LOG_MSG_COL)
		target = r->longLog();
	else if (colNum == COMMIT_COL)
		target = sha;

	// wildcard search, case insensitive
	return (target.contains(filter));
}

bool ListViewProxy::isMatch(int source_row) const {

	FileHistory* fh = d->model();
	if (fh->rowCount() <= source_row) // FIXME required to avoid an ASSERT in d->isMatch()
		return false;

	bool extFilter = (colNum == -1);
	return ((!extFilter && isMatch(fh->sha(source_row)))
	      ||( extFilter && d->isMatch(fh->sha(source_row))));
}

bool ListViewProxy::isHighlighted(int row) const {

	// FIXME row == source_row only because when
	// higlights the rows are not hidden
	return (isHighLight && isMatch(row));
}

bool ListViewProxy::filterAcceptsRow(int source_row, const QModelIndex&) const {

	return (isHighLight || isMatch(source_row));
}

int ListViewProxy::setFilter(bool isOn, bool h, SCRef fl, int cn, ShaSet* s) {

	filter = QRegExp(fl, Qt::CaseInsensitive, QRegExp::Wildcard);
	colNum = cn;
	if (s)
		shaSet = *s;

	// isHighlighted() is called also when filter is off,
	// so reset 'isHighLight' flag in that case
	isHighLight = h && isOn;

    HistoryView* lv = static_cast<HistoryView*>(parent());
	FileHistory* fh = d->model();

	if (!isOn && sourceModel()){
		lv->setModel(fh);
		setSourceModel(NULL);

	} else if (isOn && !isHighLight) {
		setSourceModel(fh); // trigger a rows scanning
		lv->setModel(this);
	}
	return (sourceModel() ? rowCount() : 0);
}

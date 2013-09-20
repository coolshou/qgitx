#include "searchedit.h"

#include <QToolButton>
#include <QStyle>
#include <QMenu>

SearchEditBase::SearchEditBase(const QPixmap& pixmap, QWidget* parent) :
    QLineEdit(parent)
{
    m_searchFilterButton = new QToolButton(this);
    m_searchFilterButton->setIcon(QIcon(pixmap));
    m_searchFilterButton->setIconSize(pixmap.size());
    m_searchFilterButton->setCursor(Qt::ArrowCursor);
    m_searchFilterButton->setStyleSheet("QToolButton { border: none; padding: 0px; padding-left: 1px; }");

    int frameWidth = style()->pixelMetric(QStyle::PM_DefaultFrameWidth);
    setStyleSheet(QString("QLineEdit { padding-left: %1px; } ").arg(pixmap.size().width() + frameWidth));
    QSize msz = minimumSizeHint();
    setMinimumSize(qMax(msz.width() ,m_searchFilterButton->sizeHint().height() + frameWidth * 2 + 2),
                   qMax(msz.height(),m_searchFilterButton->sizeHint().height() + frameWidth * 2 + 2));

    connect(m_searchFilterButton, &QToolButton::clicked, this, &SearchEditBase::searchFilterButton_clicked);

}

void SearchEditBase::resizeEvent(QResizeEvent* e) {
    Q_UNUSED(e);

    QSize sz = m_searchFilterButton->sizeHint();
    m_searchFilterButton->move(0,
                     (rect().bottom() + 1 - sz.height())/2);
}

void SearchEditBase::searchFilterButton_clicked()
{
    QMenu searchMenu;
    QAction* previouslyCheckedAction = nullptr;
    QActionGroup actionGroup(&searchMenu);

    //make the search menu items smaller, like under mac os x
    searchMenu.setStyleSheet("QMenu { font-size: 10pt; padding-bottom: 2px; padding-top: 2px; }");

    //make this action group exclusive
    //only one action can be selected
    actionGroup.setExclusive(true);

    QList<SearchFilterId> filterIds = getFilterIds();
    if(filterIds.empty()) {
        return; //no filters -> no menu
    }
    for(SearchFilterId id : filterIds) {
        QString label = getFilterLabel(id);
        QAction* menuAction = searchMenu.addAction(label);
        actionGroup.addAction(menuAction);
        menuAction->setCheckable(true);

        connect(menuAction, &QAction::triggered, [this, id, label, menuAction, &previouslyCheckedAction](){
            this->setSelectedFilter(id);
        });

        //if this is the selected filter, mark it checked
        if(id == this->getSelectedFilterId()) {
            menuAction->setChecked(true);
        }
    }

    //calculate the point where our menu should be opened
    //it should be opened at the beginning of the field, directly under it
    const QRect& geometry(this->frameGeometry());
    QPoint bottomLeft = geometry.bottomLeft();
    QPoint bottomLeftGlobal = parentWidget()->mapToGlobal(bottomLeft);

    searchMenu.exec(bottomLeftGlobal);
}

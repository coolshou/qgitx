#ifndef SEARCHEDIT_H
#define SEARCHEDIT_H

#include <QLineEdit>
#include <QIcon>

#include <memory>

template<typename T>
class SearchFilterEntry {
public:
    SearchFilterEntry(QString label, T filter) : m_label(label), m_filter(filter) {}
    QString label() { return m_label; }
    T& filter() { return m_filter; }
private:
    QString m_label;
    T m_filter;
};

class SearchFilterId {
public:
    SearchFilterId(int filterId) : m_filterId(filterId) {}
    int filterId() { return m_filterId; }

    bool operator==(const SearchFilterId& other) { return m_filterId == other.m_filterId; }
private:
    int m_filterId;
};

class SearchEditBase : public QLineEdit
{
    Q_OBJECT
public:
    explicit SearchEditBase(const QPixmap& pixmap, QWidget *parent = 0);

protected:
    void resizeEvent(QResizeEvent* e);

signals:
    void searchFilterSelected(SearchFilterId id);
    void searchMenuPrepared(QMenu& menu);

public slots:
    void searchFilterButton_clicked();

protected:
    virtual SearchFilterId getSelectedFilterId() = 0;
    virtual QList<SearchFilterId> getFilterIds() = 0;
    virtual QString getFilterLabel(SearchFilterId id) = 0;
    virtual void setSelectedFilter(SearchFilterId id) = 0;

    QToolButton* m_searchFilterButton;
};

template<typename T>
class SearchEdit : public SearchEditBase {

public:
    explicit SearchEdit(const QPixmap& pixmap, QWidget *parent = 0) : SearchEditBase(pixmap, parent), m_selectedFilterId(-1) {}

    T& selectedFilter() {
        return filter(m_selectedFilterId);
    }

    T& filter(SearchFilterId id) {
        return m_filterEntries[id.filterId()].filter();
    }
    void addFilter(QString label, T filter) {
        SearchFilterEntry<T> entry(label, filter);
        m_filterEntries.append(entry);
    }

protected:
    virtual SearchFilterId getSelectedFilterId() override {
        return m_selectedFilterId;
    }
    virtual QList<SearchFilterId> getFilterIds() override {
        QList<SearchFilterId> idList;

        for(int i = 0; i < m_filterEntries.size(); i++) {
            idList.append(SearchFilterId(i));
        }

        return idList;
    }
    virtual QString getFilterLabel(SearchFilterId id) override {
        return m_filterEntries[id.filterId()].label();
    }

    virtual void setSelectedFilter(SearchFilterId id) override {
        m_selectedFilterId = id;
    }

    QList<SearchFilterEntry<T>> m_filterEntries;
    SearchFilterId m_selectedFilterId;
};

#endif // SEARCHEDIT_H

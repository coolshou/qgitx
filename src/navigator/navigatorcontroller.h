#ifndef NAVIGATORVIEW_H
#define NAVIGATORVIEW_H

#include <functional>

#include <QObject>
#include <QHash>

class QTreeWidget;
class QTreeWidgetItem;

class NavigatorController : public QObject {
Q_OBJECT
public:
    NavigatorController(QTreeWidget& widget);

    void addBranch(const QString branchName);
    void addTag(const QString tagName);
    void addRemote(const QString remoteName);

    void removeBranch(const QString branchName);
    void removeTag(const QString tagName);
    void removeRemote(const QString remoteName);

    void markBranchAsHEAD(const QString branchName);

    void clear();

private:
    enum TreeBranch { BRANCHES, TAGS, REMOTES };
    typedef QPair<TreeBranch, const QString> Key;
    QTreeWidget& widget;
    QTreeWidgetItem* stageItem;
    QTreeWidgetItem* branchesItem;
    QTreeWidgetItem* remotesItem;
    QTreeWidgetItem* tagsItem;

    QTreeWidgetItem* currentHeadBranchItem;

    QHash<const QTreeWidgetItem*, std::function<void()> > itemSignalsMap;
    QHash<Key, QTreeWidgetItem*> itemsMap;
    QHash<QString, QTreeWidgetItem*> remotesMap;

    Key makeKey(TreeBranch treeBranch, const QString name);
    QTreeWidgetItem* createTopLevelEntry(const QString& caption);
    void addItem(Key key, QTreeWidgetItem* parent, QTreeWidgetItem* item);
    void removeItem(Key key);
    void connectQTreeWidgetItem(QTreeWidgetItem* item, std::function<void()> f);

    QPair<QString, QString> getRemoteBranchComponents(const QString& remoteBranch);

signals:
    void branchActivated(const QString& branchName);
    void tagActivated(const QString& tagName);
    void remoteActivated(const QString& remoteName);
    void stageActivated();
};

#endif

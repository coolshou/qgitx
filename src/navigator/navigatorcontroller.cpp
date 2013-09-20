#include "navigatorcontroller.h"

#include <QTreeWidget>
#include <QDebug>

NavigatorController::NavigatorController(QTreeWidget &widget) : widget(widget)
{
    //create the navigator tree structure
    //- Repository
    //|- Stage
    QTreeWidgetItem* repositoryItem = createTopLevelEntry("Repository");
    stageItem = new QTreeWidgetItem(QStringList("Stage"));
    repositoryItem->addChild(stageItem);
    //- Branches
    branchesItem = createTopLevelEntry("Branches");
    //- Remotes
    remotesItem = createTopLevelEntry("Remotes");
    //- Tags
    tagsItem = createTopLevelEntry("Tags");

    //currently selected head
    currentHeadBranchItem = nullptr;

    widget.insertTopLevelItem(0, repositoryItem);
    widget.insertTopLevelItem(1, branchesItem);
    widget.insertTopLevelItem(2, remotesItem);
    widget.insertTopLevelItem(3, tagsItem);

    QObject::connect(&widget, &QTreeWidget::itemClicked, [this](QTreeWidgetItem* item, int column) {
        Q_UNUSED(column);
        if(itemSignalsMap.contains(item)) {
            itemSignalsMap[item]();
        }
    });
}

QTreeWidgetItem* NavigatorController::createTopLevelEntry(const QString& caption)
{
    QTreeWidgetItem* item = new QTreeWidgetItem(QStringList(caption));
    item->setFlags(Qt::ItemIsEnabled);

    return item;
}

void NavigatorController::addBranch(const QString branchName)
{
    QTreeWidgetItem* item = new QTreeWidgetItem(QStringList(branchName));
    addItem(makeKey(BRANCHES, branchName), branchesItem, item);

    connectQTreeWidgetItem(item, [this, branchName](){
        emit branchActivated(branchName);
    });
}

void NavigatorController::addTag(const QString tagName)
{
    QTreeWidgetItem* item = new QTreeWidgetItem(QStringList(tagName));
    addItem(makeKey(TAGS, tagName), tagsItem, item);

    connectQTreeWidgetItem(item, [this, tagName](){
        emit tagActivated(tagName);
    });
}

void NavigatorController::addRemote(const QString remoteName)
{
    QPair<QString, QString> components = this->getRemoteBranchComponents(remoteName);
    QTreeWidgetItem* item = new QTreeWidgetItem(QStringList(components.second));
    QTreeWidgetItem* parent;

    if(remotesMap.contains(components.first))
    {
        parent = remotesMap[components.first];
    }
    else
    {
        parent = new QTreeWidgetItem(QStringList(components.first));
        remotesItem->addChild(parent);
        remotesMap[components.first] = parent;
    }

    addItem(makeKey(REMOTES, remoteName), parent, item);

    connectQTreeWidgetItem(item, [this, remoteName](){
        emit remoteActivated(remoteName);
    });
}

void NavigatorController::removeBranch(const QString branchName)
{
    removeItem(makeKey(BRANCHES, branchName));
}

void NavigatorController::removeTag(const QString tagName)
{
    removeItem(makeKey(TAGS, tagName));
}

void NavigatorController::removeRemote(const QString remoteName)
{
    removeItem(makeKey(REMOTES, remoteName));
}

void NavigatorController::clear()
{
    itemsMap.clear();;
    itemSignalsMap.clear();

    for(QTreeWidgetItem* parent : {branchesItem, tagsItem, remotesItem}) {
        QList<QTreeWidgetItem*> children;
        for(int i = 0; i < parent->childCount(); i++) {
            QTreeWidgetItem* child = parent->child(i);
            children.append(child);
        }
        for(auto child : children)
        {
            parent->removeChild(child);
        }
    }
}

void NavigatorController::markBranchAsHEAD(const QString branchName)
{
    Key key = makeKey(BRANCHES, branchName);
    if(!itemsMap.contains(key)) {
        qDebug() << "item to be marked as head branch was not found: " << key.second << "\n";
        return;
    }

    QTreeWidgetItem* item = itemsMap[key];
    QFont font = item->font(0);
    font.setBold(true);
    item->setFont(0,font);

    if(currentHeadBranchItem != nullptr)
    {
        QFont font = currentHeadBranchItem->font(0);
        font.setBold(false);
        currentHeadBranchItem->setFont(0,font);
    }

    currentHeadBranchItem = item;
}

QPair<QString, QString> NavigatorController::getRemoteBranchComponents(const QString& remoteBranch)
{
    QStringList parts = remoteBranch.split('/');
    if(parts.size() >= 2) {
        QString remote = parts[0];
        parts.removeFirst();
        QString branchName = parts.join("/");
        return QPair<QString, QString>(remote, branchName);
    }
    else {
        return QPair<QString, QString>("", remoteBranch);
    }
}

void NavigatorController::connectQTreeWidgetItem(QTreeWidgetItem* item, std::function<void()> f)
{
    itemSignalsMap.insert(item, f);
}

NavigatorController::Key NavigatorController::makeKey(TreeBranch treeBranch, const QString name)
{
    return QPair<TreeBranch, const QString>(treeBranch, name);
}

void NavigatorController::addItem(Key key, QTreeWidgetItem* parent, QTreeWidgetItem* item)
{
    parent->addChild(item);
    itemsMap.insert(key, item);
}

 void NavigatorController::removeItem(Key key)
{
    if(!itemsMap.contains(key)) {
        qDebug() << "item to be removed could not be found: " << key.first << "," << key.second << "\n";
        return;
    }

    QTreeWidgetItem* item = itemsMap[key];
    QTreeWidgetItem* parent = item->parent();
    if(parent != nullptr) {
        parent->removeChild(item);
        itemsMap.remove(key);
    }
}

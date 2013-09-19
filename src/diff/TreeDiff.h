#ifndef TREEDIFF_H
#define TREEDIFF_H

#include <QObject>

#include "diff_def.h"
#include "FileDiff.h"
#include "Hunk.h"
#include "tools/maybe.h"
#include "tools/optional.h"

class TreeDiffEntry : public QObject {
    Q_OBJECT
public:
    enum Status { ADDED, DELETED, CHANGED };
    TreeDiffEntry(QString fileName, Status status, QSharedPointer<FileDiff> fileDiff) : m_fileName(fileName), m_fileDiff(fileDiff), m_status(status) {}
    TreeDiffEntry(QString fileName, Status status, QSharedPointer<FileDiff> fileDiff, optional<QString> oldName) : m_fileName(fileName), m_fileDiff(fileDiff), m_status(status), m_oldName(oldName) {}

    bool isRenamed() { return m_oldName.available() && m_status != DELETED; }
    bool isNew() { return m_status == ADDED; }
    bool isDeleted() { return m_status == DELETED; }
    bool isChanged() { return m_status == CHANGED; }

    QString fileName() { return m_fileName; }
    QString oldName() { return *m_oldName; }
    QString displayedFileName() { if(isNew()) return fileName(); else if(isDeleted()) return oldName(); else return fileName(); }

    QSharedPointer<FileDiff> fileDiff() { return m_fileDiff; }

    Q_PROPERTY(bool isRenamed READ isRenamed)
    Q_PROPERTY(bool isNew READ isNew)
    Q_PROPERTY(bool isDeleted READ isDeleted)
    Q_PROPERTY(bool isChanged READ isChanged)
    Q_PROPERTY(QString fileName READ fileName)
    Q_PROPERTY(QString oldName READ oldName)
    Q_PROPERTY(QString displayedFileName READ displayedFileName)
    Q_PROPERTY(QSharedPointer<FileDiff> fileDiff READ fileDiff)

private:
    QString m_fileName;
    QSharedPointer<FileDiff> m_fileDiff;
    Status m_status;
    optional<QString> m_oldName;
};

class TreeDiff : public QObject
{
    Q_OBJECT
public:
    typedef QList<QSharedPointer<TreeDiffEntry>> TreeDiffEntryList;

    TreeDiff(TreeDiffEntryList entries) : m_entries(entries) {}
    static Maybe<QSharedPointer<TreeDiff> > createFromString(const QString& diffText);

    TreeDiffEntryList entries() { return m_entries; }
    Q_PROPERTY(QList<QSharedPointer<TreeDiffEntry>> entries READ entries)

private:
    static bool isExistingFile(const QString& path) { return path != "/dev/null"; }
    static QString realFileName(QString path) { static QRegularExpression prefix("^(a|b)/"); return path.replace(prefix, ""); }
    TreeDiffEntryList m_entries;
};

Q_DECLARE_METATYPE(QList<QSharedPointer<TreeDiffEntry>>)

#endif // TREEDIFF_H

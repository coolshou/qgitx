#ifndef DIFFERENCE_H
#define DIFFERENCE_H

#include <QtCore>
#include <tools/maybe.h>

#include "Hunk.h"

class FileDiff : public QObject {
    Q_OBJECT
public:
    typedef QList<QSharedPointer<Hunk>> HunksList;
    FileDiff(HunksList hunks) : m_hunks(hunks) {}

    static Maybe<QSharedPointer<FileDiff>> createFromString(QString diff);
    HunksList hunks() { return m_hunks; }

    Q_PROPERTY(QList<QSharedPointer<Hunk>> hunks READ hunks)
private:
    HunksList m_hunks;
};

Q_DECLARE_METATYPE(QList<QSharedPointer<Hunk>>)

#endif //DIFFERENCE_H

#include "Hunk.h"

#include "DiffLine.h"

void HunkBuilder::addDelete(const QString& content)
{
    QSharedPointer<DiffLine> diffLine(new DiffLine(DiffLine::DELETE, positionInOld(), nil, content));
    addLine(diffLine);
    advanceOld();
}

void HunkBuilder::addAdd(const QString& content)
{
    QSharedPointer<DiffLine> diffLine(new DiffLine(DiffLine::ADD, nil, positionInNew(), content));
    addLine(diffLine);
    advanceNew();
}

void HunkBuilder::addContext(const QString& content)
{
    QSharedPointer<DiffLine> diffLine(new DiffLine(DiffLine::CONTEXT, positionInOld(), positionInNew(), content));
    addLine(diffLine);
    advanceNew();
    advanceOld();
}

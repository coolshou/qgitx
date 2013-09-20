#ifndef HUNK_H
#define HUNK_H

#include <QtCore>

#include "tools/optional.h"
#include "diff_def.h"
#include "DiffLine.h"

class Hunk : public QObject {
    Q_OBJECT
public:
    typedef QList<QSharedPointer<DiffLine>> LinesList;

    Hunk(diffrange oldRange, diffrange newRange, LinesList lines, optional<QString> header = nil) : m_lines(lines), m_oldRange(oldRange), m_newRange(newRange), m_header(header) {}

    diffrange oldRange() { return m_oldRange; }
    diffrange newRange() { return m_newRange; }
    linenumber oldRangeStart() { return m_oldRange.start; }
    lineslenght oldRangeLength() { return m_oldRange.length; }
    linenumber newRangeStart() { return m_newRange.start; }
    lineslenght newRangeLength() { return m_newRange.length; }

    LinesList lines() { return m_lines; }

    Q_PROPERTY(QList<QSharedPointer<DiffLine>> lines READ lines)

private:
    LinesList m_lines;
    diffrange m_oldRange;
    diffrange m_newRange;
    optional<QString> m_header;
};

Q_DECLARE_METATYPE(QList<QSharedPointer<DiffLine>>)

class HunkBuilder {
public:
    HunkBuilder(diffrange oldRange, diffrange newRange) : m_oldRange(oldRange), m_newRange(newRange), m_positionInOld(oldRange.start), m_positionInNew(newRange.start) {}

    void addHeader(QString header) { m_header = header; }
    void addLine(QSharedPointer<DiffLine> line) { m_lines.append(line); }
    void addDelete(const QString& content);
    void addAdd(const QString& content);
    void addContext(const QString& content);

    void advanceOld() { m_positionInOld++; }
    void advanceNew() { m_positionInNew++; }
    QSharedPointer<Hunk> build() { return QSharedPointer<Hunk>(new Hunk(m_oldRange, m_newRange, m_lines)); }

    diffrange oldRange() { return m_oldRange; }
    diffrange newRange() { return m_newRange; }
    linenumber positionInOld() { return m_positionInOld; }
    linenumber positionInNew() { return m_positionInNew; }

private:
    diffrange m_oldRange;
    diffrange m_newRange;
    Hunk::LinesList m_lines;
    linenumber m_positionInOld;
    linenumber m_positionInNew;
    optional<QString> m_header;
};

#endif //HUNK_H

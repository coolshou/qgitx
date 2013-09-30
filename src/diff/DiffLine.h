#ifndef DIFFLINE_H
#define DIFFLINE_H

#include <QtCore>

#include "diff_def.h"
#include "tools/optional.h"

class DiffLine : public QObject {
    Q_OBJECT
public:
    enum Type {CONTEXT, ADD, DELETE};

    DiffLine(Type type, optional<linenumber> oldLineNumber, optional<linenumber> newLineNumber, const QString& content) : m_type(type), m_oldLineNumber(oldLineNumber), m_newLineNumber(newLineNumber),  m_content(content) {}

    Type type() { return m_type; }

    bool adding() { return m_type == ADD; }
    bool deleting() { return m_type == DELETE; }
    bool context() { return m_type == CONTEXT; }

    QString content() { return m_content; }
    bool hasOldLineNumber() { return m_oldLineNumber.available(); }
    bool hasNewLineNumber() { return m_newLineNumber.available(); }
    linenumber oldLineNumber() { return *m_oldLineNumber; }
    linenumber newLineNumber() { return *m_newLineNumber; }

    Q_PROPERTY(bool adding READ adding)
    Q_PROPERTY(bool deleting READ deleting)
    Q_PROPERTY(bool context READ context)
    Q_PROPERTY(QString content READ content)
    Q_PROPERTY(bool hasOldLineNumber READ hasOldLineNumber)
    Q_PROPERTY(bool hasNewLineNumber READ hasNewLineNumber)
    Q_PROPERTY(unsigned int oldLineNumber READ oldLineNumber)
    Q_PROPERTY(unsigned int newLineNumber READ newLineNumber)

protected:
    Type m_type;
    optional<linenumber> m_oldLineNumber;
    optional<linenumber> m_newLineNumber;
    QString m_content;
};

#endif //DIFFLINE_H

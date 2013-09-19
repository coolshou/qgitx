#include "FileDiff.h"

#include <memory>
#include <QRegularExpression>

#include "Hunk.h"
#include "DiffLine.h"

using namespace std;

Maybe<QSharedPointer<FileDiff>> FileDiff::createFromString(QString diff)
{
    static const QRegularExpression newLineRE = QRegularExpression(R"(\n|(\r\n))");
    static const QRegularExpression hunkStartRE = QRegularExpression(R"(@@\s+-(\d+)(,(\d+))?\s+\+(\d+)(,(\d+))?\s+@@(\s(.*))?)");
    QStringList lines = diff.split(newLineRE);
    HunksList hunks;
    unique_ptr<HunkBuilder> currentHunkBuilder = nullptr;

    linenumber l = 1;
    for(const QString& line : lines)
    {
        //if this line matches hunkStartRE, create a new hunk
        QRegularExpressionMatch hunkStartMatch = hunkStartRE.match(line);
        if(hunkStartMatch.hasMatch())
        {
            //save the old hunk
            if(currentHunkBuilder != nullptr)
            {
                hunks.append(currentHunkBuilder->build());
            }

            diffrange oldRange, newRange;
            oldRange.start = hunkStartMatch.captured(1).toInt();
            QStringRef strOldRangeLength = hunkStartMatch.capturedRef(3);
            oldRange.length = strOldRangeLength.isNull() ? 1 : strOldRangeLength.toInt();
            newRange.start = hunkStartMatch.captured(4).toInt();
            QStringRef strNewRangeLength = hunkStartMatch.capturedRef(6);
            newRange.length = strNewRangeLength.isNull() ? 1 : strNewRangeLength.toInt();

            currentHunkBuilder = unique_ptr<HunkBuilder>(new HunkBuilder(oldRange, newRange));

            //if we have a rest after the hunk, add it as context
            if(!hunkStartMatch.capturedRef(8).isNull())
            {
                currentHunkBuilder->addHeader(hunkStartMatch.captured(8));
            }
        }
        //if it is not the start of a hunk, it is either a context line or a change
        else
        {
            if(currentHunkBuilder == nullptr) {
                //if no hunk has been before, the diff is invalid,
                //as it does not start with a hunk start marker
                return Maybe<QSharedPointer<FileDiff>>::fail("start of hunk not found");
            }

            QString content = line.mid(1);
            if(line.startsWith("+")) {
                currentHunkBuilder->addAdd(content);
            }
            else if(line.startsWith("-")) {
                currentHunkBuilder->addDelete(content);
            }
            else if(line.startsWith(" ")){
                currentHunkBuilder->addContext(content);
            }
            else if(line.length() == 0) {
                continue; //empty lines are ignored
            }
            else {
                                QString error = QString("invalid format at line %1 in diff: '%2'").arg(l).arg(line);
                return Maybe<QSharedPointer<FileDiff>>::fail(error.toLatin1().constData());
            }
        }
        l++;
    }

    if(currentHunkBuilder != nullptr) {
        hunks.append(currentHunkBuilder->build());
    }
    return Maybe<QSharedPointer<FileDiff>>::ret(QSharedPointer<FileDiff>(new FileDiff(hunks)));
}

#include "TreeDiff.h"

Maybe<QSharedPointer<TreeDiff> > TreeDiff::createFromString(const QString& diffText)
{
    int idCounter = 1;
    TreeDiffEntryList entries;
    QRegularExpression fileDiffHeaderRE(
                R"(^diff[^\n]*?\n(.*?)^---\s([^\n]*?)\n^\+\+\+\s([^\n]*?)?\n)",
                QRegularExpression::DotMatchesEverythingOption | QRegularExpression::MultilineOption
                );

    auto matchIterator = fileDiffHeaderRE.globalMatch(diffText);

    //test if we have at least one match
    if(!matchIterator.hasNext())
    {
        return Maybe<QSharedPointer<TreeDiff> >::fail("incorrect diff format - no header found");
    }
    while(matchIterator.hasNext())
    {
        auto match = matchIterator.next();
        QStringList groups = match.capturedTexts();

        QString strOldFileName = realFileName(groups[2]);
        QString newFileName = realFileName(groups[3]);

        bool isDeleted = !isExistingFile(newFileName) && isExistingFile(strOldFileName);
        bool isNew = isExistingFile(newFileName) && !isExistingFile(strOldFileName);
        bool isMoved = isExistingFile(newFileName) && isExistingFile(strOldFileName) && newFileName != strOldFileName;
        optional<QString> oldFileName(strOldFileName, isMoved || isDeleted);

        TreeDiffEntry::Status status;
        if(isDeleted) {
            status = TreeDiffEntry::Status::DELETED;
        }
        else if(isNew) {
            status = TreeDiffEntry::Status::ADDED;
        }
        else {
            status = TreeDiffEntry::Status::CHANGED;
        }

        //calculate the size of the diff
        int fileDiffOffset = match.capturedStart() + match.capturedLength();
        int fileDiffLength;
        if(matchIterator.hasNext()) {
            auto nextMatch = matchIterator.peekNext();
            fileDiffLength = nextMatch.capturedStart() - match.capturedEnd();
        }
        else {
            //if we have no next file diff header, we are at the end
            //so we need to take the rest of the string
            fileDiffLength = diffText.length() - fileDiffOffset;
        }
        QString fileDiffText = diffText.mid(fileDiffOffset, fileDiffLength);
        Maybe<QSharedPointer<FileDiff>> fileDiff = FileDiff::createFromString(fileDiffText);
        if(!fileDiff) {
            //if we failed to parse the file diff, return a fail
            qDebug() << "failed to parse diff:\n"
                     << diffText
                     << "\n";
            return Maybe<QSharedPointer<TreeDiff>>::fail(fileDiff.to_error());
        }

        QSharedPointer<TreeDiffEntry> newEntry(new TreeDiffEntry(idCounter, newFileName, status, fileDiff.to_value(), oldFileName));
        entries.append(newEntry);
        ++idCounter;
    }

    return Maybe<QSharedPointer<TreeDiff>>::ret(QSharedPointer<TreeDiff>(new TreeDiff(entries)));
}



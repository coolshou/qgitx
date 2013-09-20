#ifndef DIFF_DEF_H
#define DIFF_DEF_H

typedef unsigned int linenumber;
typedef unsigned int lineslenght;

struct diffrange {
    linenumber start;
    lineslenght length;

    diffrange(linenumber start, lineslenght length) : start(start), length(length) {}
    diffrange() : start(0), length(0) {}
    bool operator==(const diffrange& other) const { return start == other.start && length == other.length; }
};

class DiffLine;
class FileDiff;
class Hunk;
class TreeDiff;

#endif // DIFF_DEF_H

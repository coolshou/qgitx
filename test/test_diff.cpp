#include <QString>
#include <QtTest>

#include "diff/diff.h"

class DiffTest : public QObject
{
    Q_OBJECT
    
public:
    DiffTest();
    
private Q_SLOTS:
    void testCase1();
};

DiffTest::DiffTest()
{
}

void DiffTest::testCase1()
{
    QString diff(
R"(@@ -1,5 +1,5 @@
+-1/1
 1/2
-2/3
 3/3
 4/4
 5/5
@@ -40,8 +40,6 @@
 40/40
 41/41
-42/-1
-43/-1
 43/42
 44/43
 45/44
@@ -52,6 +50,6 @@
 52/50
 53/51
 54/52
-55/-1
+-1/53
 56/54
 57/55)");

    auto result = FileDiff::createFromString(diff);

    QVERIFY2(result, result.to_safe_error().c_str());

    auto hunks = result.to_value()->hunks();

    QVERIFY2(hunks.size() == 3, QString("expected 3 hunks in diff, instead got %1").arg(hunks.size()).toLatin1().constData());

    QVERIFY2(hunks[0]->newRange() == diffrange(1, 5), "unexpected newRange for hunk #1");
    QVERIFY2(hunks[0]->oldRange() == diffrange(1, 5), "unexpected oldRange for hunk #1");

    QVERIFY2(hunks[1]->newRange() == diffrange(40, 6), "unexpected newRange for hunk #2");
    QVERIFY2(hunks[1]->oldRange() == diffrange(40, 8), "unexpected oldRange for hunk #2");

    QVERIFY2(hunks[2]->newRange() == diffrange(50, 6), "unexpected newRange for hunk #3");
    QVERIFY2(hunks[2]->oldRange() == diffrange(52, 6), "unexpected oldRange for hunk #3");

    for(auto& hunk : hunks)
    {

    }
}

QTEST_APPLESS_MAIN(DiffTest)

#include "test_diff.moc"

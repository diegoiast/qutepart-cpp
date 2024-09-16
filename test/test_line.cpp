#include <QDebug>
#include <QObject>
#include <QTest>

#include "qutepart.h"

class Test : public QObject {
    Q_OBJECT

  private slots:
    void doRemoveTest(const QString &text, int pos, int count, const QString &expected) {
        Qutepart::Qutepart qpart(nullptr, text);

        Qutepart::Line line(qpart.document()->firstBlock());

        line.remove(pos, count);

        QCOMPARE(line.text(), expected);
    }

    void RemoveAll() { doRemoveTest("asdfghjkl", 0, 9, ""); }

    void RemoveFirst() { doRemoveTest("asdfghjkl", 0, 1, "sdfghjkl"); }

    void RemoveLast() { doRemoveTest("asdfghjkl", 8, 1, "asdfghjk"); }

    void RemoveMid() { doRemoveTest("asdfghjkl", 2, 5, "askl"); }

    void RemoveZero() {
        doRemoveTest("asdfghjkl", 9, 0, "asdfghjkl");
        doRemoveTest("asdfghjkl", 0, 0, "asdfghjkl");
    }

    void Length() {
        Qutepart::Qutepart qpart(nullptr, "asdf\nfd");
        Qutepart::Line first(qpart.document()->firstBlock());
        QCOMPARE(first.length(), 4);

        Qutepart::Line last(qpart.document()->lastBlock());
        QCOMPARE(last.length(), 2);
    }
};

QTEST_MAIN(Test)
#include "test_line.moc"

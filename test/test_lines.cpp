#include <QDebug>
#include <QObject>
#include <QTest>

#include "qutepart.h"

class Test : public QObject {
    Q_OBJECT

  private slots:
    void Append() {
        QString text = "asdf\nfd";
        Qutepart::Qutepart qpart(nullptr, text);
        qpart.lines().append("xyz");
        QCOMPARE(qpart.toPlainText(), QString("asdf\nfd\nxyz"));

        qpart.undo();
        QCOMPARE(qpart.toPlainText(), text);
    }

    void PopAt_data() {
        QTest::addColumn<QString>("text");
        QTest::addColumn<int>("lineNumber");
        QTest::addColumn<QString>("removed");
        QTest::addColumn<QString>("expected");

        QTest::newRow("first") << "one\ntwo\nthree" << 0 << "one"
                               << "two\nthree";

        QTest::newRow("middle") << "one\ntwo\nthree" << 1 << "two"
                                << "one\nthree";

        QTest::newRow("last") << "one\ntwo\nthree" << 2 << "three"
                              << "one\ntwo";

        QTest::newRow("empty line") << "one\ntwo\nthree\n"
                                    << 3 << ""
                                    << "one\ntwo\nthree";
    }

    void PopAt() {
        QFETCH(QString, text);
        QFETCH(int, lineNumber);
        QFETCH(QString, removed);
        QFETCH(QString, expected);

        Qutepart::Qutepart qpart(nullptr, text);
        QString actualRemoved = qpart.lines().popAt(lineNumber);
        QCOMPARE(actualRemoved, removed);
        QCOMPARE(qpart.toPlainText(), expected);

        qpart.undo();
        QCOMPARE(qpart.toPlainText(), text);
    }

    void InsertAt_data() {
        QTest::addColumn<QString>("text");
        QTest::addColumn<int>("lineNumber");
        QTest::addColumn<QString>("inserted");
        QTest::addColumn<QString>("expected");

        QTest::newRow("first") << "one\ntwo\nthree" << 0 << "new"
                               << "new\none\ntwo\nthree";

        QTest::newRow("middle") << "one\ntwo\nthree" << 1 << "new"
                                << "one\nnew\ntwo\nthree";

        QTest::newRow("last") << "one\ntwo\nthree" << 2 << "new"
                              << "one\ntwo\nnew\nthree";

        QTest::newRow("after last") << "one\ntwo\nthree" << 3 << "new"
                                    << "one\ntwo\nthree\nnew";
    }

    void InsertAt() {
        QFETCH(QString, text);
        QFETCH(int, lineNumber);
        QFETCH(QString, inserted);
        QFETCH(QString, expected);

        Qutepart::Qutepart qpart(nullptr, text);
        qpart.lines().insertAt(lineNumber, inserted);
        QCOMPARE(qpart.toPlainText(), expected);

        qpart.undo();
        QCOMPARE(qpart.toPlainText(), text);
    }

    void CountAndAt() {
        Qutepart::Qutepart qpart(nullptr, "one\ntwo\nthree\n\n5\n");

        Qutepart::Lines lines = qpart.lines();

        QCOMPARE(lines.count(), 6);
        QCOMPARE(lines.at(0).text(), QString("one"));
        QCOMPARE(lines.at(2).text(), QString("three"));
        QCOMPARE(lines.at(3).text(), QString(""));
        QCOMPARE(lines.at(4).text(), QString("5"));
        QCOMPARE(lines.at(6).text(), QString(""));
    }
};

QTEST_MAIN(Test)
#include "test_lines.moc"

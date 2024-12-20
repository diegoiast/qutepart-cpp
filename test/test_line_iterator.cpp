/*
 * Copyright (C) 2018-2023 Andrei Kopats
 * Copyright (C) 2023-...  Diego Iastrubni <diegoiast@gmail.com>
 * SPDX-License-Identifier: MIT
 */

#include <QDebug>
#include <QObject>
#include <QTest>

#include "qutepart.h"

class Test : public QObject {
    Q_OBJECT

  private slots:
    void Iter() {
        Qutepart::Qutepart qpart(nullptr, "a\nb\nc");

        Qutepart::LineIterator it = qpart.lines().begin();
        QVERIFY(it != qpart.lines().end());
        QCOMPARE((*it).text(), QString("a"));

        ++it;
        QVERIFY(it != qpart.lines().end());
        QCOMPARE((*it).text(), QString("b"));

        ++it;
        QVERIFY(it != qpart.lines().end());
        QCOMPARE((*it).text(), QString("c"));

        ++it;
        QVERIFY(it == qpart.lines().end());
    }

    void ForLoop() {
        Qutepart::Qutepart qpart(nullptr, "a\nb\nc");

        QStringList expectedLines = {"a", "b", "c"};
        QStringList actualLines;

        for (auto it = qpart.lines().begin(); it != qpart.lines().end(); ++it) {
            actualLines << (*it).text();
        }

        QCOMPARE(actualLines, expectedLines);
    }

    void RangeBasedForLoop() {
        Qutepart::Qutepart qpart(nullptr, "a\nb\nc");

        QStringList expectedLines = {"a", "b", "c"};
        QStringList actualLines;
        for (const auto &line : qpart.lines()) {
            actualLines << line.text();
        }

        QCOMPARE(actualLines, expectedLines);
    }
};

QTEST_MAIN(Test)
#include "test_line_iterator.moc"

/*
 * Copyright (C) 2018-2023 Andrei Kopats
 * Copyright (C) 2023-...  Diego Iastrubni <diegoiast@gmail.com>
 * SPDX-License-Identifier: MIT
 */

#include <QDebug>
#include <QObject>
#include <QTest>

#include "qutepart.h"
#include "text_block_utils.h"

class Test : public QObject {
    Q_OBJECT

  private slots:
    void DataDrivenTest_data() {
        QTest::addColumn<QString>("text");
        QTest::addColumn<QChar>("bracket");
        QTest::addColumn<int>("blockNumber");
        QTest::addColumn<int>("column");
        QTest::addColumn<bool>("forward");
        QTest::addColumn<int>("expectedBlock");
        QTest::addColumn<int>("expectedColumn");

        QTest::newRow("Find forward") << "func(param)" << QChar('(') << 0 << 4 << true << 0 << 10;

        QTest::newRow("Find forward with nested")
            << "func(pa( )ram)" << QChar('(') << 0 << 4 << true << 0 << 13;

        QTest::newRow("Find backward") << "func(param)" << QChar('(') << 0 << 10 << false << 0 << 4;

        QTest::newRow("Find backward with nested")
            << "func(pa( )ram)" << QChar('(') << 0 << 13 << false << 0 << 4;
    }

    void DataDrivenTest() {
        QFETCH(QString, text);
        QFETCH(QChar, bracket);
        QFETCH(int, blockNumber);
        QFETCH(int, column);
        QFETCH(bool, forward);
        QFETCH(int, expectedBlock);
        QFETCH(int, expectedColumn);

        Qutepart::Qutepart qpart(nullptr, text);

        Qutepart::TextPosition pos(qpart.document()->findBlockByNumber(blockNumber), column);

        Qutepart::TextPosition foundPos;
        if (forward) {
            foundPos = Qutepart::findClosingBracketForward(bracket, pos);
        } else {
            foundPos = Qutepart::findOpeningBracketBackward(bracket, pos);
        }

        QCOMPARE(foundPos.block.blockNumber(), expectedBlock);
        QCOMPARE(foundPos.column, expectedColumn);
    }
};

QTEST_MAIN(Test)
#include "test_text_block_utils.moc"

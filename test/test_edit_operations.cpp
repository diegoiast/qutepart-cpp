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
    void SmartHome() {
        Qutepart::Qutepart qpart(nullptr, "one\n    two");

        qpart.homeAction()->trigger();
        QCOMPARE(qpart.textCursor().positionInBlock(), 0);

        qpart.homeAction()->trigger();
        QCOMPARE(qpart.textCursor().positionInBlock(), 0);

        qpart.goTo(1, 2);
        qpart.homeAction()->trigger();
        QCOMPARE(qpart.textCursor().positionInBlock(), 4);

        qpart.homeAction()->trigger();
        QCOMPARE(qpart.textCursor().positionInBlock(), 0);

        qpart.homeAction()->trigger();
        QCOMPARE(qpart.textCursor().positionInBlock(), 4);
    }

    void SmartHomeSelect() {
        Qutepart::Qutepart qpart(nullptr, "one\n    two");

        qpart.homeSelectAction()->trigger();
        QCOMPARE(qpart.textCursor().positionInBlock(), 0);
        QCOMPARE(qpart.textCursor().selectedText(), QString());

        qpart.homeSelectAction()->trigger();
        QCOMPARE(qpart.textCursor().positionInBlock(), 0);
        QCOMPARE(qpart.textCursor().selectedText(), QString());

        qpart.goTo(1, 6);
        qpart.homeSelectAction()->trigger();
        QCOMPARE(qpart.textCursor().positionInBlock(), 4);
        QCOMPARE(qpart.textCursor().selectedText(), QString("tw"));

        qpart.homeSelectAction()->trigger();
        QCOMPARE(qpart.textCursor().selectedText(), QString("    tw"));
        QCOMPARE(qpart.textCursor().positionInBlock(), 0);

        qpart.homeSelectAction()->trigger();
        QCOMPARE(qpart.textCursor().positionInBlock(), 4);
        QCOMPARE(qpart.textCursor().selectedText(), QString("tw"));
    }

    void JoinLines() {
        Qutepart::Qutepart qpart(nullptr, "one\ntwo\n    three");

        qpart.joinLinesAction()->trigger();
        QCOMPARE(qpart.toPlainText(), QString("one two\n    three"));

        qpart.joinLinesAction()->trigger();
        QCOMPARE(qpart.toPlainText(), QString("one two three"));

        qpart.joinLinesAction()->trigger();
        QCOMPARE(qpart.toPlainText(), QString("one two three"));
    }

    void JoinLinesWithSelection() {
        Qutepart::Qutepart qpart(nullptr, "one\ntwo\n    three");

        QTextCursor cursor = qpart.textCursor();
        cursor.movePosition(QTextCursor::NextCharacter);
        cursor.movePosition(QTextCursor::EndOfWord, QTextCursor::KeepAnchor);
        qpart.setTextCursor(cursor);
        QCOMPARE(qpart.textCursor().selectedText(), QString("ne"));

        qpart.joinLinesAction()->trigger();
        QCOMPARE(qpart.toPlainText(), QString("one two\n    three"));
        QCOMPARE(qpart.textCursor().selectedText(), QString("ne"));
    }

    void JoinMultipleLines() {
        Qutepart::Qutepart qpart(nullptr, "one\ntwo\n    three");

        QTextCursor cursor = qpart.textCursor();
        cursor.setPosition(13);
        cursor.setPosition(1, QTextCursor::KeepAnchor);
        qpart.setTextCursor(cursor);
        QCOMPARE(qpart.textCursor().selectedText(), QString("ne\u2029two\u2029    t"));

        qpart.joinLinesAction()->trigger();
        QCOMPARE(qpart.toPlainText(), QString("one two three"));
        QCOMPARE(qpart.textCursor().selectedText(), QString("ne two t"));
    }
};

QTEST_MAIN(Test)
#include "test_edit_operations.moc"

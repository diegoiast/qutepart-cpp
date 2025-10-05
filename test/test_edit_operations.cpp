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

        QTest::keyClick(&qpart, Qt::Key_Home);
        QCOMPARE(qpart.textCursor().positionInBlock(), 0);

        QTest::keyClick(&qpart, Qt::Key_Home);
        QCOMPARE(qpart.textCursor().positionInBlock(), 0);

        qpart.goTo(1, 2);
        QTest::keyClick(&qpart, Qt::Key_Home);
        QCOMPARE(qpart.textCursor().positionInBlock(), 4);

        QTest::keyClick(&qpart, Qt::Key_Home);
        QCOMPARE(qpart.textCursor().positionInBlock(), 0);

        QTest::keyClick(&qpart, Qt::Key_Home);
        QCOMPARE(qpart.textCursor().positionInBlock(), 4);
    }

    void SmartHomeSelect() {
        Qutepart::Qutepart qpart(nullptr, "one\n    two");

        // QTest::keyClick(&qpart, Qt::Key_Home, Qt::ShiftModifier);
        QMetaObject::invokeMethod(&qpart, "onShortcutHome", Qt::DirectConnection,
                                  Q_ARG(QTextCursor::MoveMode, QTextCursor::MoveAnchor));
        QCOMPARE(qpart.textCursor().positionInBlock(), 0);
        QCOMPARE(qpart.textCursor().selectedText(), QString());

        // QTest::keyClick(&qpart, Qt::Key_Home, Qt::ShiftModifier);
        QMetaObject::invokeMethod(&qpart, "onShortcutHome", Qt::DirectConnection,
                                  Q_ARG(QTextCursor::MoveMode, QTextCursor::MoveAnchor));
        QCOMPARE(qpart.textCursor().positionInBlock(), 0);
        QCOMPARE(qpart.textCursor().selectedText(), QString());

        qpart.goTo(1, 6);
        // QTest::keyClick(&qpart, Qt::Key_Home, Qt::ShiftModifier);
        QMetaObject::invokeMethod(&qpart, "onShortcutHome", Qt::DirectConnection,
                                  Q_ARG(QTextCursor::MoveMode, QTextCursor::MoveAnchor));
        QCOMPARE(qpart.textCursor().positionInBlock(), 4);
        QCOMPARE(qpart.textCursor().selectedText(), QString("tw"));

        // QTest::keyClick(&qpart, Qt::Key_Home, Qt::ShiftModifier);
        QMetaObject::invokeMethod(&qpart, "onShortcutHome", Qt::DirectConnection,
                                  Q_ARG(QTextCursor::MoveMode, QTextCursor::MoveAnchor));
        QCOMPARE(qpart.textCursor().selectedText(), QString("    tw"));
        QCOMPARE(qpart.textCursor().positionInBlock(), 0);

#if 0
        QTest::keyClick(&qpart, Qt::Key_Home, Qt::ShiftModifier);
        QCOMPARE(qpart.textCursor().positionInBlock(), 4);
        auto s = qpart.textCursor().selectedText();
        QCOMPARE(qpart.textCursor().selectedText(), QString("tw"));
#endif
    }

    void JoinLines() {
        Qutepart::Qutepart qpart(nullptr, "one\ntwo\n    three");

        QTest::keyClick(&qpart, Qt::Key_J, Qt::ControlModifier);
        QCOMPARE(qpart.toPlainText(), QString("one two\n    three"));

        QTest::keyClick(&qpart, Qt::Key_J, Qt::ControlModifier);
        QCOMPARE(qpart.toPlainText(), QString("one two three"));

        QTest::keyClick(&qpart, Qt::Key_J, Qt::ControlModifier);
        QCOMPARE(qpart.toPlainText(), QString("one two three"));
    }

    void JoinLinesWithSelection() {
        Qutepart::Qutepart qpart(nullptr, "one\ntwo\n    three");

        QTextCursor cursor = qpart.textCursor();
        cursor.movePosition(QTextCursor::NextCharacter);
        cursor.movePosition(QTextCursor::EndOfWord, QTextCursor::KeepAnchor);
        qpart.setTextCursor(cursor);
        QCOMPARE(qpart.textCursor().selectedText(), QString("ne"));

#if 0
        QTest::keyClick(&qpart, Qt::Key_J, Qt::ControlModifier);
        QCOMPARE(qpart.toPlainText(), QString("one two\n    three"));
        // FIXME remove space from "ne ". Actually a bug, but will fix later (I
        // hope)
        QCOMPARE(qpart.textCursor().selectedText(), QString("ne "));
#endif
    }

    void JoinMultipleLines() {
        Qutepart::Qutepart qpart(nullptr, "one\ntwo\n    three");

        QTextCursor cursor = qpart.textCursor();
        cursor.setPosition(13);
        cursor.setPosition(1, QTextCursor::KeepAnchor);
        qpart.setTextCursor(cursor);
        QCOMPARE(qpart.textCursor().selectedText(), QString("ne\u2029two\u2029    t"));

#if 0        
        QTest::keyClick(&qpart, Qt::Key_J, Qt::ControlModifier);
        QCOMPARE(qpart.toPlainText(), QString("one two three"));
        QCOMPARE(qpart.textCursor().selectedText(), QString("ne two t"));
#endif
    }
};

QTEST_MAIN(Test)
#include "test_edit_operations.moc"

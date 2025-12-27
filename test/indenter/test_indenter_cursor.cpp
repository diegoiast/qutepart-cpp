/*
 * SPDX-License-Identifier: MIT
 */

#include "base_indenter_test.h"
#include <QtTest/QtTest>

class Test : public BaseTest {
    Q_OBJECT

  private slots:
    void unindent_cursor_move() {
        // Line with 4 spaces and text "code".
        // Cursor at column 6 (between 'c' and 'o').
        // Shift+Tab -> Unindent to 0 spaces.
        // Expected cursor position: 6 - 4 = 2.

        qpart.setPlainText("    code");
        setCursorPosition(0, 6); // "    co|de"

        // Shift+Tab
        QTest::keyClick(&qpart, Qt::Key_Tab, Qt::ShiftModifier);

        QCOMPARE(qpart.toPlainText(), "code");

        int expectedCol = 2; // "co|de"
        QCOMPARE(qpart.textCursor().positionInBlock(), expectedCol);
    }

    void unindent_cursor_in_indent() {
        // "    code"
        // Cursor at 2 ("  |  code").
        // Shift+Tab -> "code".
        // Expected cursor: 0.

        qpart.setPlainText("    code");
        setCursorPosition(0, 2);

        QTest::keyClick(&qpart, Qt::Key_Tab, Qt::ShiftModifier);

        QCOMPARE(qpart.toPlainText(), "code");
        QCOMPARE(qpart.textCursor().positionInBlock(), 0);
    }

    void unindent_cursor_move_multiline() {
        // Line 1: "    code1"
        // Line 2: "    code2"
        // Selection covers both lines.
        // Shift+Tab.

        qpart.setPlainText("    code1\n    code2");
        QTextCursor cursor = qpart.textCursor();
        cursor.setPosition(6);                           // "    co|de1"
        cursor.setPosition(16, QTextCursor::KeepAnchor); // "    code1\n    co|de2"
        qpart.setTextCursor(cursor);

        QTest::keyClick(&qpart, Qt::Key_Tab, Qt::ShiftModifier);

        QCOMPARE(qpart.toPlainText(), "code1\ncode2");

        cursor = qpart.textCursor();
        QCOMPARE(cursor.selectionStart(), 2);
        QCOMPARE(cursor.selectionEnd(), 8);
    }
};

QTEST_MAIN(Test)
#include "test_indenter_cursor.moc"

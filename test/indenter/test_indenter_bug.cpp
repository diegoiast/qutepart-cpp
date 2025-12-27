/*
 * SPDX-License-Identifier: MIT
 */

#include "base_indenter_test.h"
#include <QtTest/QtTest>

class Test : public BaseTest {
    Q_OBJECT

  private slots:
    void bug_unindent_at_col0() {
        qpart.setPlainText("line1\n    line2");
        // Line 0: "line1"
        // Line 1: "    line2"

        setCursorPosition(1, 0); // Start of line2 (at column 0, before spaces)

        // Shift+Tab (Decrease Indent)
        QTest::keyClick(&qpart, Qt::Key_Tab, Qt::ShiftModifier);

        // Expected: line2 unindented, line1 untouched
        QCOMPARE(qpart.toPlainText(), "line1\nline2");
    }
};

QTEST_MAIN(Test)
#include "test_indenter_bug.moc"

/*
 * SPDX-License-Identifier: MIT
 */

#include "base_indenter_test.h"
#include <QtTest/QtTest>

class Test : public BaseTest {
    Q_OBJECT

  private slots:
    void unindent_alignment() {
        // 6 spaces. Width is default 4.
        // Expected: unindent to 4 spaces (remove 2).
        // Current Bug (suspected): unindents to 2 spaces (removes 4).

        qpart.setPlainText("      code");
        setCursorPosition(0, 0);

        // Shift+Tab
        QTest::keyClick(&qpart, Qt::Key_Tab, Qt::ShiftModifier);

        QCOMPARE(qpart.toPlainText(), "    code");
    }

    void unindent_standard() {
        // 4 spaces. Width 4.
        // Expected: unindent to 0 spaces.

        qpart.setPlainText("    code");
        setCursorPosition(0, 0);

        QTest::keyClick(&qpart, Qt::Key_Tab, Qt::ShiftModifier);

        QCOMPARE(qpart.toPlainText(), "code");
    }
};

QTEST_MAIN(Test)
#include "test_indenter_unindent.moc"

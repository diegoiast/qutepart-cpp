#include "base_indenter_test.h"

#include <QTextCursor>
#include <QtTest/QtTest>

void BaseTest::addColumns() {
    QTest::addColumn<QString>("origin");
    QTest::addColumn<CursorPos>("cursorPos");
    QTest::addColumn<QString>("input");
    QTest::addColumn<QString>("expected");
}

void BaseTest::setCursorPosition(int line, int col) {
    QTextCursor cursor(qpart.document()->findBlockByNumber(line));
    cursor.setPosition(cursor.block().position() + col);
    qpart.setTextCursor(cursor);
}

void BaseTest::enter() { QTest::keyClick(&qpart, Qt::Key_Enter); }

void BaseTest::tab() { QTest::keyClick(&qpart, Qt::Key_Tab); }

void BaseTest::type(const QString &text) { QTest::keyClicks(&qpart, text); }

void BaseTest::verifyExpected(const QString &expected) { QCOMPARE(qpart.toPlainText(), expected); }

void BaseTest::runDataDrivenTest() {
    QFETCH(QString, origin);
    QFETCH(CursorPos, cursorPos);
    QFETCH(QString, input);
    QFETCH(QString, expected);

    qpart.setPlainText(origin);

    setCursorPosition(cursorPos.first, cursorPos.second);

    if (input.startsWith("<<alignLine(")) {
        QString blockIndexStr = input.mid(QString("<<alignLine(").length(), 1);
        if (blockIndexStr == "*") {
            int initialBlockCount = qpart.document()->blockCount();
            for (int i = 0; i < initialBlockCount; i++) {
                autoIndentBlock(qpart.document()->blockCount() - initialBlockCount + i);
            }
        } else {
            int blockIndex = blockIndexStr.toInt();
            autoIndentBlock(blockIndex);
        }
    } else {
        for (auto ch = input.begin(); ch != input.end(); ++ch) {
            if (*ch == '\n') {
                enter();
            } else if (*ch == '\t') {
                tab();
            } else {
                type(*ch);
            }
        }
    }
    verifyExpected(expected);
}

void BaseTest::initTestCase() { Q_INIT_RESOURCE(qutepart_syntax_files); }

void BaseTest::init() { qpart.setPlainText(""); }

void BaseTest::autoIndentBlock(int blockIndex) {
    QTextBlock block = qpart.document()->findBlockByNumber(blockIndex);
    QTextCursor cursor(block);
    qpart.setTextCursor(cursor);
    qpart.autoIndentCurrentLine();
}

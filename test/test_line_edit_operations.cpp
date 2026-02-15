/*
 * Copyright (C) 2018-2023 Andrei Kopats
 * Copyright (C) 2023-...  Diego Iastrubni <diegoiast@gmail.com>
 * SPDX-License-Identifier: MIT
 */

#include <QDebug>
#include <QObject>
#include <QTest>
#include <QApplication>

#include "qutepart.h"

class Test : public QObject {
    Q_OBJECT

  private slots:
    void MoveDownOneLine() {
        Qutepart::Qutepart qpart(nullptr, "one\ntwo\nthree\nfour");

        qpart.moveLineDownAction()->trigger();
        QCOMPARE(qpart.toPlainText(), QString("two\none\nthree\nfour"));

        qpart.moveLineDownAction()->trigger();
        QCOMPARE(qpart.toPlainText(), QString("two\nthree\none\nfour"));

        qpart.moveLineDownAction()->trigger();
        QCOMPARE(qpart.toPlainText(), QString("two\nthree\nfour\none"));

        qpart.moveLineDownAction()->trigger();
        QCOMPARE(qpart.toPlainText(), QString("two\nthree\nfour\none"));

        qpart.undo();
        QCOMPARE(qpart.toPlainText(), QString("two\nthree\none\nfour"));
        qpart.undo();
        QCOMPARE(qpart.toPlainText(), QString("two\none\nthree\nfour"));
    }

    void MoveDownOneLineEolAtEnd() {
        Qutepart::Qutepart qpart(nullptr, "one\ntwo\nthree\nfour\n");

        qpart.moveLineDownAction()->trigger();
        QCOMPARE(qpart.toPlainText(), QString("two\none\nthree\nfour\n"));

        qpart.moveLineDownAction()->trigger();
        QCOMPARE(qpart.toPlainText(), QString("two\nthree\none\nfour\n"));

        qpart.moveLineDownAction()->trigger();
        QCOMPARE(qpart.toPlainText(), QString("two\nthree\nfour\none\n"));

        qpart.moveLineDownAction()->trigger();
        QCOMPARE(qpart.toPlainText(), QString("two\nthree\nfour\n\none"));

        qpart.moveLineDownAction()->trigger();
        QCOMPARE(qpart.toPlainText(), QString("two\nthree\nfour\n\none"));

        qpart.undo();
        QCOMPARE(qpart.toPlainText(), QString("two\nthree\nfour\none\n"));
        qpart.undo();
        QCOMPARE(qpart.toPlainText(), QString("two\nthree\none\nfour\n"));
    }

    void MoveUpOneLine() {
        Qutepart::Qutepart qpart(nullptr, "one\ntwo\nthree\nfour");

        qpart.goTo(3);

        QTest::keyClick(&qpart, Qt::Key_Right);
        QCOMPARE(qpart.textCursor().positionInBlock(), 1);

        qpart.moveLineUpAction()->trigger();
        QCOMPARE(qpart.toPlainText(), QString("one\ntwo\nfour\nthree"));

        qpart.moveLineUpAction()->trigger();
        QCOMPARE(qpart.toPlainText(), QString("one\nfour\ntwo\nthree"));

        qpart.moveLineUpAction()->trigger();
        QCOMPARE(qpart.toPlainText(), QString("four\none\ntwo\nthree"));

        qpart.moveLineUpAction()->trigger();
        QCOMPARE(qpart.toPlainText(), QString("four\none\ntwo\nthree"));

        QCOMPARE(qpart.textCursor().positionInBlock(), 1);
    }

    void MoveDownTwoLines() {
        Qutepart::Qutepart qpart(nullptr, "one\ntwo\nthree\nfour");

        QTextCursor cursor = qpart.textCursor();
        cursor.setPosition(1);
        cursor.setPosition(6, QTextCursor::KeepAnchor);
        qpart.setTextCursor(cursor);
        QCOMPARE(qpart.textCursor().selectedText(), QString("ne\u2029tw"));

        qpart.moveLineDownAction()->trigger();
        QCOMPARE(qpart.toPlainText(), QString("three\none\ntwo\nfour"));

        qpart.moveLineDownAction()->trigger();
        QCOMPARE(qpart.toPlainText(), QString("three\nfour\none\ntwo"));

        qpart.moveLineDownAction()->trigger();
        QCOMPARE(qpart.toPlainText(), QString("three\nfour\none\ntwo"));

        QCOMPARE(qpart.textCursor().selectedText(), QString("ne\u2029tw"));

        qpart.undo();
        QCOMPARE(qpart.toPlainText(), QString("three\none\ntwo\nfour"));
        qpart.undo();
        QCOMPARE(qpart.toPlainText(), QString("one\ntwo\nthree\nfour"));
    }

    void MoveUpTwoLines() {
        Qutepart::Qutepart qpart(nullptr, "one\ntwo\nthree\nfour");

        QTextCursor cursor = qpart.textCursor();
        cursor.setPosition(10);
        cursor.setPosition(16, QTextCursor::KeepAnchor);
        qpart.setTextCursor(cursor);
        QCOMPARE(qpart.textCursor().selectedText(), QString("ree\u2029fo"));

        qpart.moveLineUpAction()->trigger();
        QCOMPARE(qpart.toPlainText(), QString("one\nthree\nfour\ntwo"));

        qpart.moveLineUpAction()->trigger();
        QCOMPARE(qpart.toPlainText(), QString("three\nfour\none\ntwo"));

        qpart.moveLineUpAction()->trigger();
        QCOMPARE(qpart.toPlainText(), QString("three\nfour\none\ntwo"));

        QCOMPARE(qpart.textCursor().selectedText(), QString("ree\u2029fo"));

        qpart.undo();
        QCOMPARE(qpart.toPlainText(), QString("one\nthree\nfour\ntwo"));
        qpart.undo();
        QCOMPARE(qpart.toPlainText(), QString("one\ntwo\nthree\nfour"));
    }

    void MoveDownTwoLinesSelectionEndsAtStartOfThird() {
        Qutepart::Qutepart qpart(nullptr, "one\ntwo\nthree\nfour");

        QTextCursor cursor = qpart.textCursor();
        cursor.setPosition(0);
        cursor.setPosition(8, QTextCursor::KeepAnchor); // Start of "three"
        qpart.setTextCursor(cursor);
        QCOMPARE(qpart.textCursor().selectedText(), QString("one\u2029two\u2029"));
        QCOMPARE(qpart.textCursor().blockNumber(), 2);
        QCOMPARE(qpart.textCursor().positionInBlock(), 0);

        qpart.moveLineDownAction()->trigger();
        // Should be "three\none\ntwo\nfour"
        QCOMPARE(qpart.toPlainText(), QString("three\none\ntwo\nfour"));
    }

    void MoveUpTwoLinesSelectionEndsAtStartOfThird() {
        Qutepart::Qutepart qpart(nullptr, "one\ntwo\nthree\nfour");

        QTextCursor cursor = qpart.textCursor();
        cursor.setPosition(4);
        cursor.setPosition(14, QTextCursor::KeepAnchor); // Start of "four"
        qpart.setTextCursor(cursor);
        QCOMPARE(qpart.textCursor().selectedText(), QString("two\u2029three\u2029"));
        QCOMPARE(qpart.textCursor().blockNumber(), 3);
        QCOMPARE(qpart.textCursor().positionInBlock(), 0);

        qpart.moveLineUpAction()->trigger();
        // Should be "two\nthree\none\nfour"
        QCOMPARE(qpart.toPlainText(), QString("two\nthree\none\nfour"));
    }

    void DuplicateLine() {
        Qutepart::Qutepart qpart(nullptr, "one\ntwo\n   three\nfour");

        qpart.goTo(1, 2);
        qpart.duplicateSelectionAction()->trigger();
        QCOMPARE(qpart.toPlainText(), QString("one\ntwo\ntwo\n   three\nfour"));
        QCOMPARE(qpart.textCursor().blockNumber(), 2);
        QCOMPARE(qpart.textCursor().positionInBlock(), 2);
    }

    void DuplicateIndentedLine() {
        Qutepart::Qutepart qpart(nullptr, "one\ntwo\n   three\nfour");

        qpart.goTo(2);
        qpart.duplicateSelectionAction()->trigger();
        QCOMPARE(qpart.toPlainText(), QString("one\ntwo\n   three\n   three\nfour"));
        QCOMPARE(qpart.textCursor().blockNumber(), 3);
        QCOMPARE(qpart.textCursor().positionInBlock(), 0);
    }

    void DuplicateSelection() { // TODO move from this file
        Qutepart::Qutepart qpart(nullptr, "one\ntwo\nthree\nfour");

        QTextCursor cursor = qpart.textCursor();

        cursor.setPosition(5);
        cursor.setPosition(10, QTextCursor::KeepAnchor);
        qpart.setTextCursor(cursor);
        QCOMPARE(qpart.textCursor().selectedText(), QString("wo\u2029th"));
        qpart.duplicateSelectionAction()->trigger();
        QCOMPARE(qpart.textCursor().selectedText(), QString("wo\u2029th"));

        QCOMPARE(qpart.toPlainText(), QString("one\ntwo\nthwo\nthree\nfour"));
        QCOMPARE(qpart.textCursor().blockNumber(), 3);
        QCOMPARE(qpart.textCursor().positionInBlock(), 2);
    }

    void CutPasteLines() {
        Qutepart::Qutepart qpart(nullptr, "one\ntwo\nthree\nfour");

        QTextCursor cursor = qpart.textCursor();

        cursor.setPosition(1);
        cursor.setPosition(5, QTextCursor::KeepAnchor);
        qpart.setTextCursor(cursor);

        qpart.cutLineAction()->trigger();
        QCOMPARE(qpart.toPlainText(), QString("three\nfour"));

        QTest::keyClick(&qpart, Qt::Key_Down);
        qpart.pasteLineAction()->trigger();
        QCOMPARE(qpart.toPlainText(), QString("three\nfour\none\ntwo"));

        qpart.undo();
        QCOMPARE(qpart.toPlainText(), QString("three\nfour"));

        qpart.undo();
        QCOMPARE(qpart.toPlainText(), QString("one\ntwo\nthree\nfour"));
    }

    void CutPasteLastLines() {
        Qutepart::Qutepart qpart(nullptr, "one\ntwo\nthree\nfour");

        QTextCursor cursor = qpart.textCursor();

        cursor.setPosition(12);
        cursor.setPosition(17, QTextCursor::KeepAnchor);
        qpart.setTextCursor(cursor);

        qpart.cutLineAction()->trigger();
        QCOMPARE(qpart.toPlainText(), QString("one\ntwo"));
        QTest::keyClick(&qpart, Qt::Key_Down);
        qpart.pasteLineAction()->trigger();
        QCOMPARE(qpart.toPlainText(), QString("one\ntwo\nthree\nfour"));

        qpart.undo();
        QCOMPARE(qpart.toPlainText(), QString("one\ntwo"));

        qpart.undo();
        QCOMPARE(qpart.toPlainText(), QString("one\ntwo\nthree\nfour"));
    }

    void CopyPasteSingleLine() {
        Qutepart::Qutepart qpart(nullptr, "one\ntwo\nthree\nfour");

        QTextCursor cursor = qpart.textCursor();

        cursor.setPosition(2);
        qpart.setTextCursor(cursor);

        qpart.copyLineAction()->trigger();
        QCOMPARE(qpart.toPlainText(), QString("one\ntwo\nthree\nfour"));

        QTest::keyClick(&qpart, Qt::Key_Down);
        qpart.pasteLineAction()->trigger();
        QCOMPARE(qpart.toPlainText(), QString("one\ntwo\none\nthree\nfour"));

        qpart.undo();
        QCOMPARE(qpart.toPlainText(), QString("one\ntwo\nthree\nfour"));
    }

    void CopyPasteLastLine() {
        Qutepart::Qutepart qpart(nullptr, "one\ntwo\nthree\nfour");

        QTextCursor cursor = qpart.textCursor();

        cursor.setPosition(17);
        qpart.setTextCursor(cursor);

        qpart.copyLineAction()->trigger();

        QTest::keyClick(&qpart, Qt::Key_Up);
        QTest::keyClick(&qpart, Qt::Key_Up);
        QTest::keyClick(&qpart, Qt::Key_Up);
        qpart.pasteLineAction()->trigger();
        QCOMPARE(qpart.toPlainText(), QString("one\nfour\ntwo\nthree\nfour"));

        qpart.undo();
        QCOMPARE(qpart.toPlainText(), QString("one\ntwo\nthree\nfour"));
    }

    void DeleteLine() {
        Qutepart::Qutepart qpart(nullptr, "one\ntwo\nthree\nfour");

        QTextCursor cursor = qpart.textCursor();

        cursor.setPosition(7);
        cursor.setPosition(10, QTextCursor::KeepAnchor);
        qpart.setTextCursor(cursor);

        qpart.deleteLineAction()->trigger();
        QCOMPARE(qpart.toPlainText(), QString("one\nfour"));
        QCOMPARE(qpart.textCursor().block().text(), QString("four"));
        QCOMPARE(qpart.textCursorPosition().line, 1);
        qpart.deleteLineAction()->trigger();
        QCOMPARE(qpart.toPlainText(), QString("one"));
        QCOMPARE(qpart.textCursorPosition().line, 0);

        qpart.undo();
        QCOMPARE(qpart.toPlainText(), QString("one\nfour"));

        qpart.undo();
        QCOMPARE(qpart.toPlainText(), QString("one\ntwo\nthree\nfour"));
    }

    void DeleteFirstLine() {
        Qutepart::Qutepart qpart(nullptr, "one\ntwo\nthree\nfour");

        qpart.deleteLineAction()->trigger();
        QCOMPARE(qpart.toPlainText(), QString("two\nthree\nfour"));
        QCOMPARE(qpart.textCursorPosition().line, 0);
    }

    void InsertLineAbove() {
        Qutepart::Qutepart qpart(nullptr, " one\n  two\n   three\n    four");
        qpart.goTo(2, 4);
        qpart.insertLineAboveAction()->trigger();
        QCOMPARE(qpart.toPlainText(), QString(" one\n  two\n  \n   three\n    four"));
#if 0 // FIXME this fails. "x" gets undone with the rest of operations
        QTest::keyClicks(&qpart, "x");
        QCOMPARE(qpart.toPlainText(), QString(" one\n  two\n  x\n   three\n    four"));
        qpart.undo();
        QCOMPARE(qpart.toPlainText(), QString(" one\n  two\n  \n   three\n    four"));
#endif
        qpart.undo();
        QCOMPARE(qpart.toPlainText(), QString(" one\n  two\n   three\n    four"));
    }

    void InsertLineAboveFirst() {
        Qutepart::Qutepart qpart(nullptr, " one\n  two\n   three\n    four");
        qpart.goTo(0, 0);
        qpart.insertLineAboveAction()->trigger();
        QTest::keyClicks(&qpart, "x");
        QCOMPARE(qpart.toPlainText(), QString("x\n one\n  two\n   three\n    four"));
        qpart.undo();
        QCOMPARE(qpart.toPlainText(), QString("\n one\n  two\n   three\n    four"));
        qpart.undo();
        QCOMPARE(qpart.toPlainText(), QString(" one\n  two\n   three\n    four"));
    }

    void InsertLineBelow() {
        Qutepart::Qutepart qpart(nullptr, " one\n  two\n   three\n    four");
        qpart.goTo(2, 4);
        qpart.insertLineBelowAction()->trigger();
        QCOMPARE(qpart.toPlainText(), QString(" one\n  two\n   three\n   \n    four"));

#if 0 // FIXME this fails. "x" gets undone with the rest of operations
        QTest::keyClicks(&qpart, "x");
        QCOMPARE(qpart.toPlainText(), QString(" one\n  two\n   three\n   x\n    four"));
        qpart.undo();
        QCOMPARE(qpart.toPlainText(), QString(" one\n  two\n   three\n   \n    four"));
#endif

        qpart.undo();
        QCOMPARE(qpart.toPlainText(), QString(" one\n  two\n   three\n    four"));
    }
};

QTEST_MAIN(Test)
#include "test_line_edit_operations.moc"

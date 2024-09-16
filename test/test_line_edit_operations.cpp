#include <QDebug>
#include <QObject>
#include <QTest>

#include "qutepart.h"

class Test : public QObject {
    Q_OBJECT

  private slots:
    void MoveDownOneLine() {
        Qutepart::Qutepart qpart(nullptr, "one\ntwo\nthree\nfour");

        QTest::keyClick(&qpart, Qt::Key_Down, Qt::AltModifier);
        QCOMPARE(qpart.toPlainText(), QString("two\none\nthree\nfour"));

        QTest::keyClick(&qpart, Qt::Key_Down, Qt::AltModifier);
        QCOMPARE(qpart.toPlainText(), QString("two\nthree\none\nfour"));

        QTest::keyClick(&qpart, Qt::Key_Down, Qt::AltModifier);
        QCOMPARE(qpart.toPlainText(), QString("two\nthree\nfour\none"));

        QTest::keyClick(&qpart, Qt::Key_Down, Qt::AltModifier);
        QCOMPARE(qpart.toPlainText(), QString("two\nthree\nfour\none"));

        qpart.undo();
        QCOMPARE(qpart.toPlainText(), QString("two\nthree\none\nfour"));
        qpart.undo();
        QCOMPARE(qpart.toPlainText(), QString("two\none\nthree\nfour"));
    }

    void MoveDownOneLineEolAtEnd() {
        Qutepart::Qutepart qpart(nullptr, "one\ntwo\nthree\nfour\n");

        QTest::keyClick(&qpart, Qt::Key_Down, Qt::AltModifier);
        QCOMPARE(qpart.toPlainText(), QString("two\none\nthree\nfour\n"));

        QTest::keyClick(&qpart, Qt::Key_Down, Qt::AltModifier);
        QCOMPARE(qpart.toPlainText(), QString("two\nthree\none\nfour\n"));

        QTest::keyClick(&qpart, Qt::Key_Down, Qt::AltModifier);
        QCOMPARE(qpart.toPlainText(), QString("two\nthree\nfour\none\n"));

        QTest::keyClick(&qpart, Qt::Key_Down, Qt::AltModifier);
        QCOMPARE(qpart.toPlainText(), QString("two\nthree\nfour\n\none"));

        QTest::keyClick(&qpart, Qt::Key_Down, Qt::AltModifier);
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

        QTest::keyClick(&qpart, Qt::Key_Up, Qt::AltModifier);
        QCOMPARE(qpart.toPlainText(), QString("one\ntwo\nfour\nthree"));

        QTest::keyClick(&qpart, Qt::Key_Up, Qt::AltModifier);
        QCOMPARE(qpart.toPlainText(), QString("one\nfour\ntwo\nthree"));

        QTest::keyClick(&qpart, Qt::Key_Up, Qt::AltModifier);
        QCOMPARE(qpart.toPlainText(), QString("four\none\ntwo\nthree"));

        QTest::keyClick(&qpart, Qt::Key_Up, Qt::AltModifier);
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

        QTest::keyClick(&qpart, Qt::Key_Down, Qt::AltModifier);
        QCOMPARE(qpart.toPlainText(), QString("three\none\ntwo\nfour"));

        QTest::keyClick(&qpart, Qt::Key_Down, Qt::AltModifier);
        QCOMPARE(qpart.toPlainText(), QString("three\nfour\none\ntwo"));

        QTest::keyClick(&qpart, Qt::Key_Down, Qt::AltModifier);
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

        QTest::keyClick(&qpart, Qt::Key_Up, Qt::AltModifier);
        QCOMPARE(qpart.toPlainText(), QString("one\nthree\nfour\ntwo"));

        QTest::keyClick(&qpart, Qt::Key_Up, Qt::AltModifier);
        QCOMPARE(qpart.toPlainText(), QString("three\nfour\none\ntwo"));

        QTest::keyClick(&qpart, Qt::Key_Up, Qt::AltModifier);
        QCOMPARE(qpart.toPlainText(), QString("three\nfour\none\ntwo"));

        QCOMPARE(qpart.textCursor().selectedText(), QString("ree\u2029fo"));

        qpart.undo();
        QCOMPARE(qpart.toPlainText(), QString("one\nthree\nfour\ntwo"));
        qpart.undo();
        QCOMPARE(qpart.toPlainText(), QString("one\ntwo\nthree\nfour"));
    }

    void DuplicateLine() {
        Qutepart::Qutepart qpart(nullptr, "one\ntwo\n   three\nfour");

        QTextCursor cursor = qpart.textCursor();

        qpart.goTo(1, 2);
        QTest::keyClick(&qpart, Qt::Key_D, Qt::AltModifier);
        QCOMPARE(qpart.toPlainText(), QString("one\ntwo\ntwo\n   three\nfour"));
        QCOMPARE(qpart.textCursor().blockNumber(), 2);
        QCOMPARE(qpart.textCursor().positionInBlock(), 2);
    }

    void DuplicateIndentedLine() {
        Qutepart::Qutepart qpart(nullptr, "one\ntwo\n   three\nfour");

        QTextCursor cursor = qpart.textCursor();

        qpart.goTo(2);
        QTest::keyClick(&qpart, Qt::Key_D, Qt::AltModifier);
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
        QTest::keyClick(&qpart, Qt::Key_D, Qt::AltModifier);
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

        QTest::keyClick(&qpart, Qt::Key_X, Qt::AltModifier);
        QCOMPARE(qpart.toPlainText(), QString("three\nfour"));

        QTest::keyClick(&qpart, Qt::Key_Down);
        QTest::keyClick(&qpart, Qt::Key_V, Qt::AltModifier);
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

        QTest::keyClick(&qpart, Qt::Key_X, Qt::AltModifier);
        QCOMPARE(qpart.toPlainText(), QString("one\ntwo"));

        QTest::keyClick(&qpart, Qt::Key_Down);
        QTest::keyClick(&qpart, Qt::Key_V, Qt::AltModifier);
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

        QTest::keyClick(&qpart, Qt::Key_C, Qt::AltModifier);
        QCOMPARE(qpart.toPlainText(), QString("one\ntwo\nthree\nfour"));

        QTest::keyClick(&qpart, Qt::Key_Down);
        QTest::keyClick(&qpart, Qt::Key_V, Qt::AltModifier);
        QCOMPARE(qpart.toPlainText(), QString("one\ntwo\none\nthree\nfour"));

        qpart.undo();
        QCOMPARE(qpart.toPlainText(), QString("one\ntwo\nthree\nfour"));
    }

    void CopyPasteLastLine() {
        Qutepart::Qutepart qpart(nullptr, "one\ntwo\nthree\nfour");

        QTextCursor cursor = qpart.textCursor();

        cursor.setPosition(17);
        qpart.setTextCursor(cursor);

        QTest::keyClick(&qpart, Qt::Key_C, Qt::AltModifier);

        QTest::keyClick(&qpart, Qt::Key_Up);
        QTest::keyClick(&qpart, Qt::Key_Up);
        QTest::keyClick(&qpart, Qt::Key_Up);
        QTest::keyClick(&qpart, Qt::Key_V, Qt::AltModifier);
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

        QTest::keyClick(&qpart, Qt::Key_Delete, Qt::AltModifier);
        QCOMPARE(qpart.toPlainText(), QString("one\nfour"));
        QCOMPARE(qpart.textCursor().block().text(), QString("four"));
        QCOMPARE(qpart.textCursorPosition().line, 1);

        QTest::keyClick(&qpart, Qt::Key_Delete, Qt::AltModifier);
        QCOMPARE(qpart.toPlainText(), QString("one"));
        QCOMPARE(qpart.textCursorPosition().line, 0);

        qpart.undo();
        QCOMPARE(qpart.toPlainText(), QString("one\nfour"));

        qpart.undo();
        QCOMPARE(qpart.toPlainText(), QString("one\ntwo\nthree\nfour"));
    }

    void DeleteFirstLine() {
        Qutepart::Qutepart qpart(nullptr, "one\ntwo\nthree\nfour");

        QTextCursor cursor = qpart.textCursor();

        QTest::keyClick(&qpart, Qt::Key_Delete, Qt::AltModifier);
        QCOMPARE(qpart.toPlainText(), QString("two\nthree\nfour"));

        QCOMPARE(qpart.textCursorPosition().line, 0);
    }

    void InsertLineAbove() {
        Qutepart::Qutepart qpart(nullptr, " one\n  two\n   three\n    four");
        qpart.goTo(2, 4);
        QTest::keyClick(&qpart, Qt::Key_Return, Qt::ShiftModifier | Qt::ControlModifier);
        QCOMPARE(qpart.toPlainText(), QString(" one\n  two\n  \n   three\n    four"));
#if 0 // FIXME this fails. "x" gets undone with the rest of operations
        qDebug() << "type X" << qpart.toPlainText();
        QTest::keyClicks(&qpart, "x");
        qDebug() << "typed X" << qpart.toPlainText();
        QCOMPARE(qpart.toPlainText(), QString(" one\n  two\n  x\n   three\n    four"));
        qDebug() << "undo x";
        qpart.undo();
        qDebug() << "undo X done" << qpart.toPlainText();
        QCOMPARE(qpart.toPlainText(), QString(" one\n  two\n  \n   three\n    four"));
#endif
        qpart.undo();
        QCOMPARE(qpart.toPlainText(), QString(" one\n  two\n   three\n    four"));
    }

    void InsertLineAboveFirst() {
        Qutepart::Qutepart qpart(nullptr, " one\n  two\n   three\n    four");
        qpart.goTo(0, 0);
        QTest::keyClick(&qpart, Qt::Key_Return, Qt::ShiftModifier | Qt::ControlModifier);
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
        QTest::keyClick(&qpart, Qt::Key_Return, Qt::ControlModifier);
        QCOMPARE(qpart.toPlainText(), QString(" one\n  two\n   three\n   \n    four"));

#if 0 // FIXME this fails. "x" gets undone with the rest of operations
        qDebug() << "type X" << qpart.toPlainText();
        QTest::keyClicks(&qpart, "x");
        qDebug() << "typed X" << qpart.toPlainText();
        QCOMPARE(qpart.toPlainText(), QString(" one\n  two\n   three\n   x\n    four"));
        qDebug() << "undo x";
        qpart.undo();
        qDebug() << "undo X done" << qpart.toPlainText();
        QCOMPARE(qpart.toPlainText(), QString(" one\n  two\n   three\n   \n    four"));
#endif

        qpart.undo();
        QCOMPARE(qpart.toPlainText(), QString(" one\n  two\n   three\n    four"));
    }
};

QTEST_MAIN(Test)
#include "test_line_edit_operations.moc"

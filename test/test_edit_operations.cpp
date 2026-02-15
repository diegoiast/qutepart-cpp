/*
 * Copyright (C) 2018-2023 Andrei Kopats
 * Copyright (C) 2023-...  Diego Iastrubni <diegoiast@gmail.com>
 * SPDX-License-Identifier: MIT
 */

#include <QDebug>
#include <QObject>
#include <QTest>

#include "qutepart.h"
#include "hl/syntax_highlighter.h"

class Test : public QObject {
    Q_OBJECT

  private slots:
    void initTestCase() {
        Q_INIT_RESOURCE(qutepart_syntax_files);
    }

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

    void ToggleCommentInScript() {
        QString text = "<html>\n"
                       "<script>\n"
                       "var i = 11;\n"
                       "</script>\n"
                       "</html>";
        Qutepart::Qutepart qpart(nullptr, text);
        qpart.setHighlighter("html.xml");

        // Force highlight
        auto hl = qpart.findChild<Qutepart::SyntaxHighlighter *>();
        if (hl) {
            hl->rehighlight();
        }

        // Line 2 (0-indexed) is "var i = 11;"
        qpart.goTo(2, 0);

        qpart.toggleCommentAction()->trigger();

        QString lineText = qpart.lines().at(2).text().trimmed();

        // Should be JavaScript comment
        QVERIFY(lineText.startsWith("//"));
        QVERIFY(!lineText.contains("<!--"));
    }

    void ToggleCommentPHP_CSS() {
        QString text = "<html>\n"
                       "<style>\n"
                       "body { color: red; }\n"
                       "</style>\n"
                       "<?php echo 1; ?>\n"
                       "</html>";
        Qutepart::Qutepart qpart(nullptr, text);
        qpart.setHighlighter("html-php.xml");

        auto hl = qpart.findChild<Qutepart::SyntaxHighlighter *>();
        if (hl) {
            hl->rehighlight();
        }

        // Line 0: <html> -> should use HTML comments <!-- -->
        qpart.goTo(0, 0);
        qpart.toggleCommentAction()->trigger();
        QString lineText = qpart.lines().at(0).text().trimmed();
        QCOMPARE(lineText, QString("<!--<html>-->"));

        // Line 2: body { color: red; } -> should use CSS comments /* */
        qpart.goTo(2, 0);
        qpart.toggleCommentAction()->trigger();
        lineText = qpart.lines().at(2).text().trimmed();
        QCOMPARE(lineText, QString("/*body { color: red; }*/"));

        // Line 4: <?php echo 1; ?> -> should use PHP comments (//)
        qpart.goTo(4, 6); // Inside <?php
        qpart.toggleCommentAction()->trigger();
        lineText = qpart.lines().at(4).text().trimmed();
        QVERIFY(lineText.contains("//"));

        // Multiline PHP comment
        qpart.undo();
        QTextCursor cursor = qpart.textCursor();
        cursor.setPosition(qpart.document()->findBlockByNumber(4).position() + 6);
        cursor.setPosition(qpart.document()->findBlockByNumber(4).position() + 13, QTextCursor::KeepAnchor);
        qpart.setTextCursor(cursor);
        qpart.toggleCommentAction()->trigger();
        lineText = qpart.lines().at(4).text().trimmed();
        // Since it's a selection within a line, it might use multiline markers /* */
        QVERIFY(lineText.contains("/*") && lineText.contains("*/"));
    }

    void ToggleCommentHTML_Style() {
        QString text = "<html>\n"
                       "\n"
                       "<script>\n"
                       "var 1 = \"11\";\n"
                       "</script>\n"
                       "\n"
                       "<style>\n"
                       "a: { baa: 0}\n"
                       "</style>\n"
                       "\n"
                       "aaa\n"
                       "</html>";
        Qutepart::Qutepart qpart(nullptr, text);
        qpart.setHighlighter("html.xml");

        auto hl = qpart.findChild<Qutepart::SyntaxHighlighter *>();
        if (hl) {
            hl->rehighlight();
        }

        // According to user example:
        // <html> (line 0)
        // (line 1)
        // <script> (line 2)
        // var 1 = "11"; (line 3)
        // </script> (line 4)
        // (line 5)
        // <style> (line 6)
        // a: { baa: 0} (line 7)
        // </style> (line 8)
        
        qpart.goTo(7, 0);
        qpart.toggleCommentAction()->trigger();
        QString lineText = qpart.lines().at(7).text().trimmed();
        
        // It should NOT be an HTML comment
        QVERIFY(!lineText.startsWith("<!--"));
        // Based on css.xml, it should be /* */
        QVERIFY(lineText.startsWith("/*") && lineText.endsWith("*/"));
    }

    void ToggleCommentPascal() {
        Qutepart::Qutepart qpart(nullptr, "program Hello;\n"
                                          "begin\n"
                                          "  writeln('Hello');\n"
                                          "end.");
        qpart.setHighlighter("pascal.xml");

        auto hl = qpart.findChild<Qutepart::SyntaxHighlighter *>();
        if (hl) {
            hl->rehighlight();
        }

        // Single line
        qpart.goTo(2, 0);
        qpart.toggleCommentAction()->trigger();
        QString lineText = qpart.lines().at(2).text().trimmed();
        // Pascal usually uses // or { } or (* *)
        // Checking common one
        QVERIFY(lineText.startsWith("//") || lineText.startsWith("{") || lineText.startsWith("(*"));

        // Multiline selection
        qpart.undo();
        QTextCursor cursor = qpart.textCursor();
        cursor.setPosition(qpart.document()->findBlockByNumber(1).position());
        cursor.setPosition(qpart.document()->findBlockByNumber(3).position(), QTextCursor::KeepAnchor);
        qpart.setTextCursor(cursor);

        qpart.toggleCommentAction()->trigger();
        lineText = qpart.toPlainText();
        // Should have comments on multiple lines or wrapped
        // Qutepart's toggleComment handles multiple lines by commenting each line if single line comment available
        QVERIFY(qpart.lines().at(1).text().trimmed().startsWith("//") ||
                qpart.lines().at(1).text().trimmed().startsWith("{") ||
                qpart.lines().at(1).text().trimmed().startsWith("(*") ||
                lineText.contains("{"));
    }
};

QTEST_MAIN(Test)
#include "test_edit_operations.moc"

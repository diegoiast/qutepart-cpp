/*
 * Copyright (C) 2023-...  Diego Iastrubni <diegoiast@gmail.com>
 * SPDX-License-Identifier: MIT
 */

#include <QObject>
#include <QTest>
#include <QDebug>

#include "qutepart.h"
#include "hl/syntax_highlighter.h"

using namespace Qutepart;

class Test : public QObject {
    Q_OBJECT

  private slots:
    void initTestCase() {
        Q_INIT_RESOURCE(qutepart_syntax_files);
    }

    void MoveFoldedDownOverFoldedOneStep() {
        QString text = "/*\n" // 0
                       "  comment1\n" // 1
                       "*/\n" // 2
                       "/*\n" // 3
                       "  comment2\n" // 4
                       "*/"; // 5
        Qutepart::Qutepart qpart(nullptr, text);
        qpart.setHighlighter("cpp.xml");

        auto hl = qpart.findChild<SyntaxHighlighter *>();
        if (hl) {
            hl->rehighlight();
        }

        auto doc = qpart.document();

        // Fold all blocks
        qpart.foldBlock(0);
        qpart.foldBlock(3);

        // Select line 0 only
        qpart.goTo(0, 0);

        qDebug() << "Moving line 0 down";
        qpart.moveLineDownAction()->trigger();
        QVERIFY(doc->findBlockByNumber(0).isVisible());
        QVERIFY(doc->findBlockByNumber(1).isVisible());
        QVERIFY(doc->findBlockByNumber(2).isVisible());
        
        // Now move line 2 down. Target is line 3 (/* of block2).
        qpart.goTo(2, 0);
        qDebug() << "Moving line 2 down";
        qpart.moveLineDownAction()->trigger();
        
        // Target is line 3. Line 3 is start of block2. It should unfold.
        QVERIFY(doc->findBlockByNumber(4).isVisible()); // comment2 should be visible
    }

    void MoveFoldedOverFolded() {
        QString text = "/*\n"
                       "  comment1\n"
                       "*/\n"
                       "/*\n"
                       "  comment2\n"
                       "*/\n"
                       "/*\n"
                       "  comment3\n"
                       "*/";
        Qutepart::Qutepart qpart(nullptr, text);
        qpart.setHighlighter("cpp.xml");

        auto hl = qpart.findChild<SyntaxHighlighter *>();
        if (hl) {
            hl->rehighlight();
        }

        auto doc = qpart.document();

        // Fold all blocks
        qpart.foldBlock(0);
        qpart.foldBlock(3);
        qpart.foldBlock(6);

        // Select comment1 (0, 1, 2)
        QTextCursor cursor(doc->findBlockByNumber(0));
        cursor.movePosition(QTextCursor::NextBlock, QTextCursor::KeepAnchor, 2);
        cursor.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
        qpart.setTextCursor(cursor);

        qDebug() << "Moving comment1 (0,1,2) down 3 times";
        qpart.moveLineDownAction()->trigger();
        qpart.moveLineDownAction()->trigger();
        qpart.moveLineDownAction()->trigger();

        // comment2 (original 3,4,5) should now be at (0,1,2)
        // comment1 (original 0,1,2) should now be at (3,4,5)

        QCOMPARE(doc->findBlockByNumber(0).text(), QString("/*"));
        QCOMPARE(doc->findBlockByNumber(1).text(), QString("  comment2"));
        QCOMPARE(doc->findBlockByNumber(2).text(), QString("*/"));

        // comment2 should have been unfolded
        QVERIFY(doc->findBlockByNumber(0).isVisible());
        QVERIFY(doc->findBlockByNumber(1).isVisible());
        QVERIFY(doc->findBlockByNumber(2).isVisible());
    }
};

QTEST_MAIN(Test)
#include "test_move_lines_folding.moc"

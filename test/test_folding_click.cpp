/*
 * Copyright (C) 2026-...  Diego Iastrubni <diegoiast@gmail.com>
 * SPDX-License-Identifier: MIT
 */

#include <QObject>
#include <QTest>
#include <QDebug>

#include "qutepart.h"
#include "side_areas.h"
#include "hl/syntax_highlighter.h"
#include "text_block_user_data.h"

using namespace Qutepart;

class Test : public QObject {
    Q_OBJECT

  private slots:
    void initTestCase() {
        Q_INIT_RESOURCE(qutepart_syntax_files);
    }

    void FoldOnlyOnIndicators() {
        QString text = "void Hello() {\n"
                       "    line2\n"
                       "    line3\n"
                       "}";
        Qutepart::Qutepart qpart(nullptr, text);
        qpart.setHighlighter("cpp.xml");

        // Force highlight
        auto hl = qpart.findChild<SyntaxHighlighter*>();
        if (hl) {
            hl->rehighlight();
        }

        auto doc = qpart.document();
        
        // Find FoldingArea. It's private, but we can find it via children
        auto foldingArea = qpart.findChild<FoldingArea*>();
        QVERIFY(foldingArea != nullptr);

        // We can use cursorRect to find the position of a block
        auto y0 = qpart.QPlainTextEdit::cursorRect(QTextCursor(doc->findBlockByNumber(0))).center().y();
        auto y1 = qpart.QPlainTextEdit::cursorRect(QTextCursor(doc->findBlockByNumber(1))).center().y();

        // Click on line 0 (should have - indicator)
        QTest::mousePress(foldingArea, Qt::LeftButton, Qt::NoModifier, QPoint(5, y0));
        QVERIFY(doc->findBlockByNumber(0).userData() != nullptr);
        auto data0 = static_cast<TextBlockUserData *>(doc->findBlockByNumber(0).userData());
        QVERIFY(data0->folding.folded); // Should be folded

        // Unfold
        QTest::mousePress(foldingArea, Qt::LeftButton, Qt::NoModifier, QPoint(5, y0));
        QVERIFY(!data0->folding.folded);

        // Click on line 1 (should NOT have indicator, but level is > 0)
        QTest::mousePress(foldingArea, Qt::LeftButton, Qt::NoModifier, QPoint(5, y1));
        auto data1 = static_cast<TextBlockUserData *>(doc->findBlockByNumber(1).userData());
        QVERIFY(!data1->folding.folded); // Should NOT be folded

        // Click on line 3 (index 3) (closing brace, should NOT have indicator)
        auto y3 = qpart.QPlainTextEdit::cursorRect(QTextCursor(doc->findBlockByNumber(3))).center().y();
        QTest::mousePress(foldingArea, Qt::LeftButton, Qt::NoModifier, QPoint(5, y3));
        auto data3 = static_cast<TextBlockUserData *>(doc->findBlockByNumber(3).userData());
        QVERIFY(!data3->folding.folded); // Should NOT be folded
    }
};

QTEST_MAIN(Test)
#include "test_folding_click.moc"

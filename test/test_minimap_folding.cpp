/*
 * Copyright (C) 2026-...  Diego Iastrubni <diegoiast@gmail.com>
 * SPDX-License-Identifier: MIT
 */

#include <QtTest>
#include <QApplication>

#include "qutepart/qutepart.h"
#include "side_areas.h"
#include "hl/syntax_highlighter.h"
#include "text_block_user_data.h"

class TestMinimapFolding : public QObject {
    Q_OBJECT
  private slots:
    void initTestCase() {
        Q_INIT_RESOURCE(qutepart_syntax_files);
    }

    void FoldUpdatesVisibleLineCount() {
        QString text = "void Hello() {\n"
                       "    line2\n"
                       "    line3\n"
                       "    line4\n"
                       "    line5\n"
                       "}";
        Qutepart::Qutepart qpart(nullptr, text);
        qpart.setHighlighter("cpp.xml");
        auto hl = qpart.findChild<Qutepart::SyntaxHighlighter *>();
        if (hl) {
            hl->rehighlight();
        }

        qpart.resize(1200, 800);
        qpart.show();
        qpart.setMinimapVisible(true);
        QApplication::processEvents();

        auto *mini = qpart.findChild<Qutepart::Minimap *>();
        QVERIFY(mini != nullptr);
        QVERIFY(mini->isVisible());

        // Baseline: every line visible
        QCOMPARE(mini->visibleLineCount(), 6);

        // Fold the function body (hides lines 1..4, leaves braces visible)
        qpart.toggleFold(0);
        QApplication::processEvents();
        QCOMPARE(mini->visibleLineCount(), 2);

        // Unfold: all lines visible again
        qpart.toggleFold(0);
        QApplication::processEvents();
        QCOMPARE(mini->visibleLineCount(), 6);
    }
};

QTEST_MAIN(TestMinimapFolding)
#include "test_minimap_folding.moc"
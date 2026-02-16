/*
 * Copyright (C) 2023-...  Diego Iastrubni <diegoiast@gmail.com>
 * SPDX-License-Identifier: MIT
 */

#include <QDebug>
#include <QObject>
#include <QTest>

#include "qutepart.h"
#include "text_block_user_data.h"
#include "hl/syntax_highlighter.h"

class Test : public QObject {
    Q_OBJECT

  private slots:
    void initTestCase() {
        Q_INIT_RESOURCE(qutepart_syntax_files);
    }

    void FoldStruct() {
        QString text = "struct GitStatusEntry {\n"
                       "    QString filename;\n"
                       "    GitFileStatus status;\n"
                       "    bool checked = false;\n"
                       "};";
        Qutepart::Qutepart qpart(nullptr, text);
        qpart.setHighlighter("cpp.xml");

        // Force highlight
        auto hl = qpart.findChild<Qutepart::SyntaxHighlighter*>();
        if (hl) {
            hl->rehighlight();
        }

        auto doc = qpart.document();

        // Fold line 0
        qpart.foldBlock(0);

        // Expectation: lines 1, 2, 3 are hidden. line 0 and 4 are visible.
        QVERIFY(doc->findBlockByNumber(0).isVisible());
        QVERIFY(!doc->findBlockByNumber(1).isVisible());
        QVERIFY(!doc->findBlockByNumber(2).isVisible());
        QVERIFY(!doc->findBlockByNumber(3).isVisible());
        QVERIFY(doc->findBlockByNumber(4).isVisible());
    }

    void FoldPascalFunction() {
        QString text = "procedure Hello;\n"
                       "begin\n"
                       "  writeln('Hello');\n"
                       "  writeln('World');\n"
                       "end;";
        Qutepart::Qutepart qpart(nullptr, text);
        qpart.setHighlighter("pascal.xml");

        auto hl = qpart.findChild<Qutepart::SyntaxHighlighter*>();
        if (hl) {
            hl->rehighlight();
        }

        auto doc = qpart.document();
        
        // In pascal.xml, 'begin' starts Region1 and 'end' ends it.
        // Line 1 is 'begin'.
        qpart.foldBlock(1);

        QVERIFY(doc->findBlockByNumber(0).isVisible()); // procedure
        QVERIFY(doc->findBlockByNumber(1).isVisible()); // begin
        QVERIFY(!doc->findBlockByNumber(2).isVisible()); // writeln
        QVERIFY(!doc->findBlockByNumber(3).isVisible()); // writeln
        QVERIFY(doc->findBlockByNumber(4).isVisible()); // end;
    }

    void FoldCSharpFunction() {
        QString text = "void Hello() {\n"
                       "    Console.WriteLine(\"Hello\");\n"
                       "    Console.WriteLine(\"World\");\n"
                       "}";
        Qutepart::Qutepart qpart(nullptr, text);
        qpart.setHighlighter("cs.xml");

        auto hl = qpart.findChild<Qutepart::SyntaxHighlighter*>();
        if (hl) {
            hl->rehighlight();
        }

        auto doc = qpart.document();
        
        // Line 0 has '{' which starts block1.
        qpart.foldBlock(0);

        QVERIFY(doc->findBlockByNumber(0).isVisible());
        QVERIFY(!doc->findBlockByNumber(1).isVisible());
        QVERIFY(!doc->findBlockByNumber(2).isVisible());
        QVERIFY(doc->findBlockByNumber(3).isVisible());
    }

    void FoldCppMultilineComment() {
        QString text = "/*\n"
                       " * line 1\n"
                       " * line 2\n"
                       " */\n"
                       "int i = 0;";
        Qutepart::Qutepart qpart(nullptr, text);
        qpart.setHighlighter("cpp.xml");

        auto hl = qpart.findChild<Qutepart::SyntaxHighlighter*>();
        if (hl) {
            hl->rehighlight();
        }

        auto doc = qpart.document();

        // Line 0 is '/*' which starts the Comment region.
        qpart.foldBlock(0);

        // Expectation: lines 1, 2 are hidden. line 0, 3, 4 are visible.
        QVERIFY(doc->findBlockByNumber(0).isVisible());
        QVERIFY(!doc->findBlockByNumber(1).isVisible());
        QVERIFY(!doc->findBlockByNumber(2).isVisible());
        QVERIFY(doc->findBlockByNumber(3).isVisible());
        QVERIFY(doc->findBlockByNumber(4).isVisible());
    }

    void FoldTopLevelBlocks() {
        QString text = "namespace N {\n"
                       "class C {\n"
                       "    void f() {\n"
                       "    }\n"
                       "};\n"
                       "}";
        Qutepart::Qutepart qpart(nullptr, text);
        qpart.setHighlighter("cpp.xml");

        auto hl = qpart.findChild<Qutepart::SyntaxHighlighter*>();
        if (hl) {
            hl->rehighlight();
        }

        auto doc = qpart.document();

        // Fold namespace N (line 0)
        qpart.foldBlock(0);

        // Expectation: lines 1-4 are hidden. line 0 and 5 are visible.
        QVERIFY(doc->findBlockByNumber(0).isVisible());
        QVERIFY(!doc->findBlockByNumber(1).isVisible());
        QVERIFY(!doc->findBlockByNumber(2).isVisible());
        QVERIFY(!doc->findBlockByNumber(3).isVisible());
        QVERIFY(!doc->findBlockByNumber(4).isVisible());
        QVERIFY(doc->findBlockByNumber(5).isVisible());

        // Unfold
        qpart.unfoldBlock(0);
        QVERIFY(doc->findBlockByNumber(1).isVisible());

        // Fold class C (line 1)
        qpart.foldBlock(1);
        QVERIFY(doc->findBlockByNumber(1).isVisible());
        QVERIFY(!doc->findBlockByNumber(2).isVisible());
        QVERIFY(!doc->findBlockByNumber(3).isVisible());
        QVERIFY(doc->findBlockByNumber(4).isVisible());
    }

    void FoldFirstMultilineComment() {
        QString text = "/*\n"
                       " * License header\n"
                       " * Line 2\n"
                       " */\n"
                       "\n"
                       "int main() {\n"
                       "    return 0;\n"
                       "}";
        Qutepart::Qutepart qpart(nullptr, text);
        qpart.setHighlighter("cpp.xml");

        auto hl = qpart.findChild<Qutepart::SyntaxHighlighter*>();
        if (hl) {
            hl->rehighlight();
        }

        auto doc = qpart.document();

        // Fold the license comment (line 0)
        qpart.foldBlock(0);

        // Expect line 0 and 3 and 4 to be visible. line 1, 2 hidden.
        QVERIFY(doc->findBlockByNumber(0).isVisible());
        QVERIFY(!doc->findBlockByNumber(1).isVisible());
        QVERIFY(!doc->findBlockByNumber(2).isVisible());
        QVERIFY(doc->findBlockByNumber(3).isVisible());
        QVERIFY(doc->findBlockByNumber(4).isVisible());
    }

    void FoldTopLevelFunction() {
        QString text = "void standaloneFunction() {\n"
                       "    int a = 1;\n"
                       "    int b = 2;\n"
                       "}";
        Qutepart::Qutepart qpart(nullptr, text);
        qpart.setHighlighter("cpp.xml");

        auto hl = qpart.findChild<Qutepart::SyntaxHighlighter*>();
        if (hl) {
            hl->rehighlight();
        }

        auto doc = qpart.document();

        // Fold line 0
        qpart.foldBlock(0);

        // Expectation: lines 1, 2 are hidden. line 0, 3 are visible.
        QVERIFY(doc->findBlockByNumber(0).isVisible());
        QVERIFY(!doc->findBlockByNumber(1).isVisible());
        QVERIFY(!doc->findBlockByNumber(2).isVisible());
        QVERIFY(doc->findBlockByNumber(3).isVisible());
    }

    void FoldComplexTopLevel() {
        QString text = "/*\n"
                       " * Copyright (C) 2018-2023 Andrei Kopats\n"
                       " * Copyright (C) 2023-...  Diego Iastrubni <diegoiast@gmail.com>\n"
                       " * SPDX-License-Identifier: MIT\n"
                       " */\n"
                       "\n"
                       "#include <QApplication>\n"
                       "#include <QPainter>\n"
                       "#include <QScrollBar>\n"
                       "#include <QTextBlock>\n"
                       "\n"
                       "#include \"qutepart.h\"\n"
                       "#include \"text_block_flags.h\"\n"
                       "#include \"theme.h\"\n"
                       "\n"
                       "namespace Qutepart {\n"
                       "\n"
                       "const int LEFT_LINE_NUM_MARGIN = 5;\n"
                       "const int RIGHT_LINE_NUM_MARGIN = 3;\n"
                       "\n"
                       "auto static blendColors(const QColor &color1, const QColor &color2, float r = 0.5) -> QColor {\n"
                       "    foo();\n"
                       "}\n"
                       "\n"
                       "auto static countDigits(int n) -> int {\n"
                       "    return 0;\n"
                       "}\n"
                       "\n"
                       "} // namespace\n"
                       "\n"
                       "auto blabla() -> void {\n"
                       "    auto a = 12;\n"
                       "}";
        Qutepart::Qutepart qpart(nullptr, text);
        qpart.setHighlighter("cpp.xml");

        auto hl = qpart.findChild<Qutepart::SyntaxHighlighter *>();
        if (hl) {
            hl->rehighlight();
        }

        auto doc = qpart.document();

        // Fold the top-level license comment (line 0)
        qpart.foldBlock(0);
        QVERIFY(doc->findBlockByNumber(0).isVisible());
        QVERIFY(!doc->findBlockByNumber(1).isVisible());
        QVERIFY(!doc->findBlockByNumber(2).isVisible());
        QVERIFY(!doc->findBlockByNumber(3).isVisible());
        QVERIFY(doc->findBlockByNumber(4).isVisible());
        QVERIFY(doc->findBlockByNumber(5).isVisible());
        qpart.unfoldBlock(0);

        // Fold the namespace (line 15)
        qpart.foldBlock(15);
        QVERIFY(doc->findBlockByNumber(15).isVisible());
        QVERIFY(!doc->findBlockByNumber(16).isVisible());
        QVERIFY(!doc->findBlockByNumber(17).isVisible());
        QVERIFY(!doc->findBlockByNumber(20).isVisible());
        QVERIFY(!doc->findBlockByNumber(27).isVisible());
        QVERIFY(doc->findBlockByNumber(28).isVisible());
        qpart.unfoldBlock(15);

        // Fold blendColors (line 20)
        qpart.foldBlock(20);
        QVERIFY(doc->findBlockByNumber(20).isVisible());
        QVERIFY(!doc->findBlockByNumber(21).isVisible());
        QVERIFY(doc->findBlockByNumber(22).isVisible());
        qpart.unfoldBlock(20);

        // Fold blabla (line 30)
        qpart.foldBlock(30);
        QVERIFY(doc->findBlockByNumber(30).isVisible());
        QVERIFY(!doc->findBlockByNumber(31).isVisible());
        QVERIFY(doc->findBlockByNumber(32).isVisible());
    }

    void FoldWithTwoToplevelItems() {
        QString text = "/*\n"
                       " license... \n"
                       "*/\n"
                       "namespace fdfd {\n"
                       "  int bar() {\n"
                       "  }\n"
                       "}";
        Qutepart::Qutepart qpart(nullptr, text);
        qpart.setHighlighter("cpp.xml");

        auto hl = qpart.findChild<Qutepart::SyntaxHighlighter *>();
        if (hl) {
            hl->rehighlight();
        }

        auto doc = qpart.document();

        // Trigger Alt+0
        qpart.foldTopLevelAction()->trigger();

        // Both top-level items should be folded.
        QVERIFY(doc->findBlockByNumber(0).isVisible()); // /*
        QVERIFY(!doc->findBlockByNumber(1).isVisible()); //  license... 
        QVERIFY(doc->findBlockByNumber(2).isVisible()); // */
        
        QVERIFY(doc->findBlockByNumber(3).isVisible()); // namespace
        QVERIFY(!doc->findBlockByNumber(4).isVisible()); //   int bar() {
        QVERIFY(!doc->findBlockByNumber(5).isVisible()); //   }
        QVERIFY(doc->findBlockByNumber(6).isVisible()); // }
    }

    void FoldAlt0() {
        QString text = "/*\n"
                       " * Copyright (C) 2018-2023 Andrei Kopats\n"
                       " * Copyright (C) 2023-...  Diego Iastrubni <diegoiast@gmail.com>\n"
                       " * SPDX-License-Identifier: MIT\n"
                       " */\n"
                       "\n"
                       "#include <QApplication>\n"
                       "#include <QPainter>\n"
                       "#include <QScrollBar>\n"
                       "#include <QTextBlock>\n"
                       "\n"
                       "#include \"qutepart.h\"\n"
                       "#include \"text_block_flags.h\"\n"
                       "#include \"theme.h\"\n"
                       "\n"
                       "namespace Qutepart {\n"
                       "\n"
                       "const int LEFT_LINE_NUM_MARGIN = 5;\n"
                       "const int RIGHT_LINE_NUM_MARGIN = 3;\n"
                       "\n"
                       "auto static blendColors(const QColor &color1, const QColor &color2, float r = 0.5) -> QColor {\n"
                       "    foo();\n"
                       "}\n"
                       "\n"
                       "auto static countDigits(int n) -> int {\n"
                       "    return 0;\n"
                       "}\n"
                       "\n"
                       "} // namespace\n"
                       "\n"
                       "auto blabla() -> void {\n"
                       "    auto a = 12;\n"
                       "}";
        Qutepart::Qutepart qpart(nullptr, text);
        qpart.setHighlighter("cpp.xml");

        auto hl = qpart.findChild<Qutepart::SyntaxHighlighter *>();
        if (hl) {
            hl->rehighlight();
        }

        auto doc = qpart.document();

        // Trigger Alt+0
        qpart.foldTopLevelAction()->trigger();

        // Verify top-level comment is folded (closing line is visible)
        QVERIFY(doc->findBlockByNumber(0).isVisible());
        QVERIFY(!doc->findBlockByNumber(1).isVisible());
        QVERIFY(doc->findBlockByNumber(4).isVisible());

        // Verify namespace is folded (closing brace is visible)
        QVERIFY(doc->findBlockByNumber(15).isVisible());
        QVERIFY(!doc->findBlockByNumber(16).isVisible());
        QVERIFY(!doc->findBlockByNumber(27).isVisible());
        QVERIFY(doc->findBlockByNumber(28).isVisible());

        // Verify blabla is folded (closing brace is visible)
        QVERIFY(doc->findBlockByNumber(30).isVisible());
        QVERIFY(!doc->findBlockByNumber(31).isVisible());
        QVERIFY(doc->findBlockByNumber(32).isVisible());
    }
};

QTEST_MAIN(Test)
#include "test_folding_struct.moc"

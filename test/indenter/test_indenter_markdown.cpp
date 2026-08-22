/*
 * Copyright (C) 2023-...  Diego Iastrubni <diegoiast@gmail.com>
 * SPDX-License-Identifier: MIT
 */

#include <QtTest/QtTest>

#include "base_indenter_test.h"

class Test : public BaseTest {
    Q_OBJECT

  private slots:
    void init() override {
        BaseTest::init();

        qpart.setHighlighter("markdown.xml");
        qpart.setIndentAlgorithm(Qutepart::INDENT_ALG_MARKDOWN);
        qpart.setIndentWidth(4);
        qpart.setLineLengthEdge(80);
    }

    void UnorderedList() { runDataDrivenTest(); }

    void UnorderedList_data() {
        addColumns();

        QTest::newRow("dash_continues") << "- item\n"
                                        << std::make_pair(0, 6) << "\n"
                                        << "- item\n"
                                           "- \n";

        QTest::newRow("star_continues") << "* item\n"
                                        << std::make_pair(0, 6) << "\n"
                                        << "* item\n"
                                           "* \n";

        QTest::newRow("plus_continues") << "+ item\n"
                                        << std::make_pair(0, 6) << "\n"
                                        << "+ item\n"
                                           "+ \n";

        QTest::newRow("indented_dash") << "   - item\n"
                                       << std::make_pair(0, 9) << "\n"
                                       << "   - item\n"
                                          "   - \n";

        QTest::newRow("empty_item_stops") << "- \n"
                                          << std::make_pair(0, 2) << "\n"
                                          << "- \n"
                                             "\n";
    }

    void TaskList() { runDataDrivenTest(); }

    void TaskList_data() {
        addColumns();

        QTest::newRow("unchecked_continues") << "- [ ] task\n"
                                             << std::make_pair(0, 10) << "\n"
                                             << "- [ ] task\n"
                                                "- [ ] \n";

        QTest::newRow("checked_becomes_unchecked") << "- [x] done\n"
                                                   << std::make_pair(0, 10) << "\n"
                                                   << "- [x] done\n"
                                                      "- [ ] \n";

        QTest::newRow("uppercase_X_becomes_unchecked") << "- [X] done\n"
                                                       << std::make_pair(0, 10) << "\n"
                                                       << "- [X] done\n"
                                                          "- [ ] \n";
    }

    void OrderedList() { runDataDrivenTest(); }

    void OrderedList_data() {
        addColumns();

        QTest::newRow("increments") << "1. item\n"
                                    << std::make_pair(0, 7) << "\n"
                                    << "1. item\n"
                                       "2. \n";

        QTest::newRow("increments_from_3") << "3. item\n"
                                           << std::make_pair(0, 7) << "\n"
                                           << "3. item\n"
                                              "4. \n";

        QTest::newRow("empty_item_stops") << "1. \n"
                                          << std::make_pair(0, 3) << "\n"
                                          << "1. \n"
                                             "\n";
    }

    void Blockquote() { runDataDrivenTest(); }

    void Blockquote_data() {
        addColumns();

        QTest::newRow("single_depth") << "> text\n"
                                      << std::make_pair(0, 6) << "\n"
                                      << "> text\n"
                                         "> \n";

        QTest::newRow("double_depth") << ">> text\n"
                                      << std::make_pair(0, 7) << "\n"
                                      << ">> text\n"
                                         ">> \n";

        QTest::newRow("empty_exits") << "> \n"
                                     << std::make_pair(0, 2) << "\n"
                                     << "> \n"
                                        "\n";

        QTest::newRow("nested_empty_exits") << ">> \n"
                                            << std::make_pair(0, 3) << "\n"
                                            << ">> \n"
                                               "\n";
    }

    void Header() { runDataDrivenTest(); }

    void Header_data() {
        addColumns();

        QTest::newRow("h1_no_propagation") << "# Title\n"
                                           << std::make_pair(0, 7) << "\n"
                                           << "# Title\n"
                                              "\n";

        QTest::newRow("h2_no_propagation") << "## Section\n"
                                           << std::make_pair(0, 10) << "\n"
                                           << "## Section\n"
                                              "\n";
    }

    void DefaultIndent() { runDataDrivenTest(); }

    void DefaultIndent_data() {
        addColumns();

        QTest::newRow("preserves_leading_spaces") << "    text\n"
                                                  << std::make_pair(0, 8) << "\n"
                                                  << "    text\n"
                                                     "    \n";

        QTest::newRow("no_indent_plain") << "plain text\n"
                                         << std::make_pair(0, 10) << "\n"
                                         << "plain text\n"
                                            "\n";
    }
    // A soft wrapped line continues the item, it does not start a new one
    void WrappedUnorderedList() {
        runWrapTest("- aaaa bbbb", 10, " c",
                    "- aaaa \n"
                    "  bbbb c");
    }

    void WrappedTaskList() {
        runWrapTest("- [ ] aaaa bbbb", 10, " c",
                    "- [ ] aaaa \n"
                    "      bbbb c");
    }

    void WrappedOrderedList() {
        runWrapTest("10. aaaa bbbb", 10, " c",
                    "10. aaaa \n"
                    "    bbbb c");
    }

    void WrappedIndentedList() {
        runWrapTest("  - aaaa bbbb", 10, " c",
                    "  - aaaa \n"
                    "    bbbb c");
    }

    void WrappedBlockquote() {
        runWrapTest("> aaaa bbbb", 10, " c",
                    "> aaaa \n"
                    "> bbbb c");
    }

    void WrappedHeader() {
        runWrapTest("# aaaa bbbb", 10, " c",
                    "# aaaa \n"
                    "bbbb c");
    }

    void WrappedPlainText() {
        runWrapTest("aaaa bbbb", 8, " c",
                    "aaaa \n"
                    "bbbb c");
    }

    // The real world case: an item reaching the default line length edge
    void OverflowAtDefaultEdge() {
        auto const origin = "1. " + QString(76, 'x') + " ";
        QCOMPARE(origin.length(), 80);

        qpart.setLineLengthEdge(80);
        qpart.setPlainText(origin);
        setCursorPosition(0, origin.length());
        type("foobar");
        verifyExpected(origin + "\n"
                                "   foobar");
    }

    // Shift+Enter opens a line inside the item, without starting a new one
    void ShiftEnterInList() { runShiftEnterTest(); }

    void ShiftEnterInList_data() {
        addColumns();

        QTest::newRow("unordered") << "- item\n" << std::make_pair(0, 6) << "\n"
                                   << "- item\n"
                                      "  \n";

        QTest::newRow("ordered") << "10. item\n" << std::make_pair(0, 8) << "\n"
                                 << "10. item\n"
                                    "    \n";

        QTest::newRow("task") << "- [ ] task\n" << std::make_pair(0, 10) << "\n"
                              << "- [ ] task\n"
                                 "      \n";

        QTest::newRow("nested") << "  - item\n" << std::make_pair(0, 8) << "\n"
                                << "  - item\n"
                                   "    \n";

        QTest::newRow("blockquote") << "> quote\n" << std::make_pair(0, 7) << "\n"
                                    << "> quote\n"
                                       "> \n";

        QTest::newRow("header") << "# Title\n" << std::make_pair(0, 7) << "\n"
                                << "# Title\n"
                                   "\n";

        QTest::newRow("plain_keeps_indent") << "    text\n" << std::make_pair(0, 8) << "\n"
                                            << "    text\n"
                                               "    \n";

        // The line is already a continuation, its alignment is kept
        QTest::newRow("continuation") << "- item\n"
                                         "  more\n"
                                      << std::make_pair(1, 6) << "\n"
                                      << "- item\n"
                                         "  more\n"
                                         "  \n";

        // Text after the cursor moves down and keeps the alignment
        QTest::newRow("splits_line") << "- one two\n" << std::make_pair(0, 6) << "\n"
                                     << "- one \n"
                                        "  two\n";
    }

    // A new item is started only by Enter, Shift+Enter never does
    void ShiftEnterThenTypingStaysInItem() {
        qpart.setPlainText("- item");
        setCursorPosition(0, 6);
        shiftEnter();
        type("more");
        verifyExpected("- item\n"
                       "  more");
    }

    // Enter at the end of a wrapped item continues the list, the marker of the
    // item is a few lines above
    void EnterAfterWrappedUnorderedList() {
        runWrapTest("- aaaa bbbb", 10, " c\n",
                    "- aaaa \n"
                    "  bbbb c\n"
                    "- ");
    }

    void EnterAfterWrappedOrderedList() {
        runWrapTest("10. aaaa bbbb", 10, " c\n",
                    "10. aaaa \n"
                    "    bbbb c\n"
                    "11. ");
    }

    void EnterAfterWrappedTaskList() {
        runWrapTest("- [ ] aaaa bbbb", 10, " c\n",
                    "- [ ] aaaa \n"
                    "      bbbb c\n"
                    "- [ ] ");
    }

    void EnterAfterWrappedBlockquote() {
        runWrapTest("> aaaa bbbb", 10, " c\n",
                    "> aaaa \n"
                    "> bbbb c\n"
                    "> ");
    }

    // Two continuation lines still resolve to the item they belong to
    void EnterAfterTwiceWrappedList() {
        runWrapTest("- aaaa bbbb", 10, " cccc dd\n",
                    "- aaaa \n"
                    "  bbbb \n"
                    "  cccc dd\n"
                    "- ");
    }

    void EnterAfterWrappedNestedList() {
        runWrapTest("  - aaaa bbbb", 10, " c\n",
                    "  - aaaa \n"
                    "    bbbb c\n"
                    "  - ");
    }

    // An indented paragraph which continues no list keeps its own indent
    void EnterAfterIndentedParagraph() {
        qpart.setPlainText("plain\n"
                           "  indented");
        setCursorPosition(1, 10);
        enter();
        verifyExpected("plain\n"
                       "  indented\n"
                       "  ");
    }

    // The continuation line keeps its alignment when it wraps again
    void WrappedListContinuation() {
        runWrapTest("- aaaa bbbb", 10, " cccc dd",
                    "- aaaa \n"
                    "  bbbb \n"
                    "  cccc dd");
    }

  private:
    // Same as runDataDrivenTest(), but the input is typed with Shift pressed
    void runShiftEnterTest() {
        QFETCH(QString, origin);
        QFETCH(CursorPos, cursorPos);
        QFETCH(QString, expected);

        qpart.setPlainText(origin);
        setCursorPosition(cursorPos.first, cursorPos.second);
        shiftEnter();
        verifyExpected(expected);
    }

    // Type `text` at the end of `origin`, with soft wrapping happening at `edge`
    void runWrapTest(const QString &origin, int edge, const QString &text,
                     const QString &expected) {
        qpart.setLineLengthEdge(edge);
        qpart.setPlainText(origin);
        setCursorPosition(0, origin.length());
        for (auto ch = text.begin(); ch != text.end(); ++ch) {
            if (*ch == '\n') {
                enter();
            } else {
                type(*ch);
            }
        }
        verifyExpected(expected);
    }
};

QTEST_MAIN(Test)
#include "test_indenter_markdown.moc"

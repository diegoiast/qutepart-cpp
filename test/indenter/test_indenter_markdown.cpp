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
};

QTEST_MAIN(Test)
#include "test_indenter_markdown.moc"

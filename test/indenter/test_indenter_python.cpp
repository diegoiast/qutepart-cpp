/*
 * Copyright (C) 2018-2023 Andrei Kopats
 * Copyright (C) 2023-...  Diego Iastrubni <diegoiast@gmail.com>
 * SPDX-License-Identifier: MIT
 */

#include <QtTest/QtTest>

#include "base_indenter_test.h"

class Test : public BaseTest {
    Q_OBJECT

  private slots:
    void python_data() {
        addColumns();
        QTest::newRow("dedent return 1") << "def some_function():\n"
                                            "  return"
                                         << std::make_pair(1, 8) << "\npass"
                                         << "def some_function():\n"
                                            "  return\n"
                                            "pass";

        QTest::newRow("dedent continue") << "while True:\n"
                                            "  continue"
                                         << std::make_pair(1, 10) << "\npass"
                                         << "while True:\n"
                                            "  continue\n"
                                            "pass";

        QTest::newRow("keep indent 1") << "def some_function(param, param2):\n"
                                          "  a = 5\n"
                                          "  b = 7"
                                       << std::make_pair(2, 7) << "\npass"
                                       << "def some_function(param, param2):\n"
                                          "  a = 5\n"
                                          "  b = 7\n"
                                          "  pass";

        QTest::newRow("keep indent 2") << "class my_class():\n"
                                          "  def my_fun():\n"
                                          "    print \"Foo\"\n"
                                          "    print 3"
                                       << std::make_pair(3, 11) << "\npass"
                                       << "class my_class():\n"
                                          "  def my_fun():\n"
                                          "    print \"Foo\"\n"
                                          "    print 3\n"
                                          "    pass";

        QTest::newRow("keep indent 3") << "while True:\n"
                                          "  returnFunc()\n"
                                          "  myVar = 3"
                                       << std::make_pair(2, 11) << "\npass"
                                       << "while True:\n"
                                          "  returnFunc()\n"
                                          "  myVar = 3\n"
                                          "  pass";

        QTest::newRow("keep indent 4")
            << "def some_function():" << std::make_pair(0, 20) << "\npass\n\npass"
            << "def some_function():\n"
               "  pass\n"
               "\n"
               "pass";

        QTest::newRow("dedent raise") << "try:\n"
                                         "  raise"
                                      << std::make_pair(1, 7) << "\nexcept:"
                                      << "try:\n"
                                         "  raise\n"
                                         "except:";

        QTest::newRow("indent colon 1")
            << "def some_function(param, param2):" << std::make_pair(0, 33) << "\npass"
            << "def some_function(param, param2):\n"
               "  pass";

        QTest::newRow("indent colon 2") << "def some_function(1,\n"
                                           "                  2):"
                                        << std::make_pair(1, 21) << "\npass"
                                        << "def some_function(1,\n"
                                           "                  2):\n"
                                           "  pass";

        QTest::newRow("indent colon 3") // do not indent colon if hanging indentation used
            << "    a = {1:" << std::make_pair(0, 11) << "\nx"
            << "    a = {1:\n"
               "         x";

        QTest::newRow("dedent pass") << "def some_function():\n"
                                        "  pass"
                                     << std::make_pair(1, 6) << "\npass"
                                     << "def some_function():\n"
                                        "  pass\n"
                                        "pass";

        QTest::newRow("dedent return 2") << "def some_function():\n"
                                            "  return"
                                         << std::make_pair(1, 8) << "\npass"
                                         << "def some_function():\n"
                                            "  return\n"
                                            "pass";

        QTest::newRow("autoindent after empty") << "while True:\n"
                                                   "   returnFunc()\n"
                                                   "\n"
                                                   "   myVar = 3"
                                                << std::make_pair(2, 0) << "\n\tx"
                                                << "while True:\n"
                                                   "   returnFunc()\n"
                                                   "\n"
                                                   "   x\n"
                                                   "   myVar = 3";

        QTest::newRow("hanging indentation")
            << "     return func (something," << std::make_pair(0, 28) << "\nx"
            << "     return func (something,\n"
               "                  x";

        QTest::newRow("hanging indentation 2") << "     return func (\n"
                                                  "        something,"
                                               << std::make_pair(1, 18) << "\nx"
                                               << "     return func (\n"
                                                  "        something,\n"
                                                  "        x";

        QTest::newRow("hanging indentation 3") << "      a = func (\n"
                                                  "          something)"
                                               << std::make_pair(1, 20) << "\nx"
                                               << "      a = func (\n"
                                                  "          something)\n"
                                                  "      x";

        QTest::newRow("hanging indentation 4") << "     return func(a,\n"
                                                  "                 another_func(1,\n"
                                                  "                              2),"
                                               << std::make_pair(2, 33) << "\nx"
                                               << "     return func(a,\n"
                                                  "                 another_func(1,\n"
                                                  "                              2),\n"
                                                  "                 x";

        QTest::newRow("hanging indentation 5") << "     return func(another_func(1,\n"
                                                  "                              2),"
                                               << std::make_pair(1, 33) << "\nx"
                                               << "     return func(another_func(1,\n"
                                                  "                              2),\n"
                                                  "                 x";

        QTest::newRow("indent brace") << "a = {" << std::make_pair(0, 5) << "\nx"
                                      << "a = {\n"
                                         "  x";

        QTest::newRow("indent bracket") << "a = [" << std::make_pair(0, 5) << "\nx"
                                        << "a = [\n"
                                           "  x";

        QTest::newRow("indent return paren") << "  return (" << std::make_pair(0, 10) << "\nx"
                                             << "  return (\n"
                                                "    x";
    }

    void linecomment_data() {
        addColumns();

        // Pressing enter on a '#' line continues the comment.
        QTest::newRow("linecomment1") << "# foo\n"
                                    << std::make_pair(0, 5) << "\nbar"
                                    << "# foo\n"
                                       "# bar\n";

        // ...keeping the indentation of the comment.
        QTest::newRow("linecomment2") << "  # foo\n"
                                    << std::make_pair(0, 7) << "\nbar"
                                    << "  # foo\n"
                                       "  # bar\n";

        // ...keeping the extra space after the comment marker.
        QTest::newRow("linecomment3") << "#foo bar\n"
                                    << std::make_pair(0, 8) << "\nbaz"
                                    << "#foo bar\n"
                                       "#baz\n";

        // An empty '#' line also continues.
        QTest::newRow("linecomment4") << "  #\n"
                                    << std::make_pair(0, 3) << "\nfoo"
                                    << "  #\n"
                                       "  # foo\n";

        // Inside a function.
        QTest::newRow("linecomment5") << "def f():\n"
                                        "    # comment\n"
                                        "\n"
                                    << std::make_pair(1, 13) << "\nnext"
                                    << "def f():\n"
                                       "    # comment\n"
                                       "    # next\n"
                                       "\n";

        // A comment behind code is not continued - it is a trailing comment.
        QTest::newRow("linecomment6") << "x = 1  # comment\n"
                                    << std::make_pair(0, 16) << "\nnext"
                                    << "x = 1  # comment\n"
                                       "next\n";

        // An empty line in between breaks the continuation.
        QTest::newRow("linecomment7") << "# foo\n"
                                        "\n"
                                    << std::make_pair(1, 0) << "\nbar"
                                    << "# foo\n"
                                       "\n"
                                       "bar\n";

        // Code following a comment is not part of the comment.
        QTest::newRow("linecomment8") << "# foo\n"
                                        "bar\n"
                                    << std::make_pair(1, 3) << "\nbaz"
                                    << "# foo\n"
                                       "bar\n"
                                       "baz\n";

        // A comment marker inside a string does not continue.
        QTest::newRow("linecomment9") << "s = \"# not a comment\"\n"
                                    << std::make_pair(0, 21) << "\nbaz"
                                    << "s = \"# not a comment\"\n"
                                       "baz\n";
    }

    void linecomment() {
        qpart.setHighlighter("python.xml");
        qpart.setIndentAlgorithm(Qutepart::INDENT_ALG_PYTHON);
        qpart.setIndentWidth(4);
        runDataDrivenTest();
    }

    void python() {
        qpart.setIndentAlgorithm(Qutepart::INDENT_ALG_PYTHON);
        qpart.setIndentWidth(2);
        runDataDrivenTest();
    }
};

QTEST_MAIN(Test)
#include "test_indenter_python.moc"

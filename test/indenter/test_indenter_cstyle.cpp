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
    void init() override {
        BaseTest::init();

        qpart.setIndentAlgorithm(Qutepart::INDENT_ALG_CSTYLE);
        qpart.setHighlighter("cpp.xml");
        qpart.setIndentWidth(2);
    }

    void Top() { runDataDrivenTest(); }

    void Top_data() {
        addColumns();

        QTest::newRow("top1") << "int {\n"
                                 ""
                              << std::make_pair(0, 5) << "\nok"
                              << "int {\n"
                                 "  ok\n";

        QTest::newRow("top2") << "\n"
                                 "int {\n"
                                 ""
                              << std::make_pair(1, 5) << "\nok"
                              << "\n"
                                 "int {\n"
                                 "  ok\n";

        QTest::newRow("top3") << "// should always indent after opening brace\n"
                                 "int {\n"
                                 ""
                              << std::make_pair(1, 5) << "\nok"
                              << "// should always indent after opening brace\n"
                                 "int {\n"
                                 "  ok\n";

        QTest::newRow("top4") << "// should always indent after opening brace\n"
                                 "\n"
                                 "int {\n"
                                 ""
                              << std::make_pair(2, 5) << "\nok"
                              << "// should always indent after opening brace\n"
                                 "\n"
                                 "int {\n"
                                 "  ok\n";

        QTest::newRow("top5") << ";\n"
                                 "int {\n"
                                 ""
                              << std::make_pair(1, 5) << "\nok"
                              << ";\n"
                                 "int {\n"
                                 "  ok\n";

        QTest::newRow("top6") << ":\n"
                                 "int {\n"
                                 ""
                              << std::make_pair(1, 5) << "\nok"
                              << ":\n"
                                 "int {\n"
                                 "  ok\n";

        QTest::newRow("top7") << "}\n"
                                 "int {\n"
                                 ""
                              << std::make_pair(1, 5) << "\nok"
                              << "}\n"
                                 "int {\n"
                                 "  ok\n";

        QTest::newRow("top8") << "{\n"
                                 "int {\n"
                                 ""
                              << std::make_pair(1, 5) << "\nok"
                              << "{\n"
                                 "int {\n"
                                 "  ok\n";

        QTest::newRow("top9") << ")\n"
                                 "int {\n"
                                 ""
                              << std::make_pair(1, 5) << "\nok"
                              << ")\n"
                                 "int {\n"
                                 "  ok\n";

        QTest::newRow("top10") << "(\n"
                                  "int {\n"
                                  ""
                               << std::make_pair(1, 5) << "\nok"
                               << "(\n"
                                  "int {\n"
                                  "  ok\n";

        QTest::newRow("top11") << "n\n"
                                  "int {\n"
                                  ""
                               << std::make_pair(1, 5) << "\nok"
                               << "n\n"
                                  "int {\n"
                                  "  ok\n";

        QTest::newRow("top12") << ";\n"
                                  "\n"
                                  "int {\n"
                                  ""
                               << std::make_pair(2, 5) << "\nok"
                               << ";\n"
                                  "\n"
                                  "int {\n"
                                  "  ok\n";

        QTest::newRow("top13") << ":\n"
                                  "\n"
                                  "int {\n"
                                  ""
                               << std::make_pair(2, 5) << "\nok"
                               << ":\n"
                                  "\n"
                                  "int {\n"
                                  "  ok\n";

        QTest::newRow("top14") << "}\n"
                                  "\n"
                                  "int {\n"
                                  ""
                               << std::make_pair(2, 5) << "\nok"
                               << "}\n"
                                  "\n"
                                  "int {\n"
                                  "  ok\n";

        QTest::newRow("top15") << "{\n"
                                  "\n"
                                  "int {\n"
                                  ""
                               << std::make_pair(2, 5) << "\nok"
                               << "{\n"
                                  "\n"
                                  "int {\n"
                                  "  ok\n";

        QTest::newRow("top16") << ")\n"
                                  "\n"
                                  "int {\n"
                                  ""
                               << std::make_pair(2, 5) << "\nok"
                               << ")\n"
                                  "\n"
                                  "int {\n"
                                  "  ok\n";

        QTest::newRow("top17") << "(\n"
                                  "\n"
                                  "int {\n"
                                  ""
                               << std::make_pair(2, 5) << "\nok"
                               << "(\n"
                                  "\n"
                                  "int {\n"
                                  "  ok\n";

        QTest::newRow("top18") << "n\n"
                                  "\n"
                                  "int {\n"
                                  ""
                               << std::make_pair(2, 5) << "\nok"
                               << "n\n"
                                  "\n"
                                  "int {\n"
                                  "  ok\n";

        QTest::newRow("top19") << "// leading comment should not cause second "
                                  "line to be indented\n"
                                  ""
                               << std::make_pair(0, 62) << "\nok"
                               << "// leading comment should not cause second "
                                  "line to be indented\n"
                                  "ok\n";
    }

    void If() { runDataDrivenTest(); }

    void If_data() {
        addColumns();

        QTest::newRow("if1") << "int fla() {\n"
                                "  if (0<x)\n"
                                ""
                             << std::make_pair(1, 10) << "\nok"
                             << "int fla() {\n"
                                "  if (0<x)\n"
                                "    ok\n";

        QTest::newRow("if2") << "int fla() {\n"
                                "  if (0<x)\n"
                                "    x = 0;\n"
                                ""
                             << std::make_pair(2, 10) << "\nok"
                             << "int fla() {\n"
                                "  if (0<x)\n"
                                "    x = 0;\n"
                                "  ok\n";

        QTest::newRow("if3") << "int fla() {\n"
                                "  if (0<x)\n"
                                "    x = 0;\n"
                                "  else\n"
                                ""
                             << std::make_pair(3, 6) << "\nok"
                             << "int fla() {\n"
                                "  if (0<x)\n"
                                "    x = 0;\n"
                                "  else\n"
                                "    ok\n";

        QTest::newRow("if4") << "int fla() {\n"
                                "  if (0<x)\n"
                                "    x = 0;\n"
                                "  else\n"
                                "    x = -x;\n"
                                ""
                             << std::make_pair(4, 11) << "\nok"
                             << "int fla() {\n"
                                "  if (0<x)\n"
                                "    x = 0;\n"
                                "  else\n"
                                "    x = -x;\n"
                                "  ok\n";

        QTest::newRow("if5") << "int fla() {\n"
                                "  if (0<x)\n"
                                "    x = 0;\n"
                                "  else if (y<x)\n"
                                ""
                             << std::make_pair(3, 15) << "\nok"
                             << "int fla() {\n"
                                "  if (0<x)\n"
                                "    x = 0;\n"
                                "  else if (y<x)\n"
                                "    ok\n";

        QTest::newRow("if6") << "int fla() {\n"
                                "  if (0<x) x(0);\n"
                                ""
                             << std::make_pair(1, 16) << "\nok"
                             << "int fla() {\n"
                                "  if (0<x) x(0);\n"
                                "  ok\n";

        QTest::newRow("if7") << "int fla() {\n"
                                "  if (0<x)\n"
                                "    x = 0;\n"
                                "  else x(-1);\n"
                                ""
                             << std::make_pair(3, 13) << "\nok"
                             << "int fla() {\n"
                                "  if (0<x)\n"
                                "    x = 0;\n"
                                "  else x(-1);\n"
                                "  ok\n";

        QTest::newRow("if8") << "int fla() {\n"
                                "  if (0<x)\n"
                                "    x = 0;\n"
                                "  else if (y<x)\n"
                                "    y = x;\n"
                                ""
                             << std::make_pair(4, 10) << "\nok"
                             << "int fla() {\n"
                                "  if (0<x)\n"
                                "    x = 0;\n"
                                "  else if (y<x)\n"
                                "    y = x;\n"
                                "  ok\n";

        QTest::newRow("if9") << "int fla() {\n"
                                "  if (0<x)\n"
                                "    x = 0;\n"
                                "  else if (y<x) y = x;\n"
                                ""
                             << std::make_pair(3, 22) << "\nok"
                             << "int fla() {\n"
                                "  if (0<x)\n"
                                "    x = 0;\n"
                                "  else if (y<x) y = x;\n"
                                "  ok\n";

        QTest::newRow("if10") << "if () {}" << std::make_pair(0, 5) << "\n"
                              << "if ()\n"
                                 "{}";

        QTest::newRow("if11") << "  if (0<x) {\n"
                                 "    x = 0;\n"
                                 "  }\n"
                                 "\n"
                                 "text;\n"
                                 ""
                              << std::make_pair(4, 0) << "\n\t"
                              << "  if (0<x) {\n"
                                 "    x = 0;\n"
                                 "  }\n"
                                 "\n"
                                 "\n"
                                 "  text;\n";
#if 0
        QTest::newRow("if12") << "if (0<x)\n"
                                 "text;\n"
                                 ""
                              << std::make_pair(8, 0) << ""
                              << "if (0<x)\n"
                                 "    text;a\n";
        QTest::newRow("if_qutepart1") << "  if (1)\n"
                                         "  {\n"
                                         "     a = 7;"
                                      << std::make_pair(2, 11) << "\nx"
                                      << "  if (1)\n"
                                         "  {\n"
                                         "     a = 7;\n"
                                         "     x";
#endif
    }

    void While() { runDataDrivenTest(); }

    void While_data() {
        addColumns();

        QTest::newRow("while1") << "int fla() {\n"
                                   "  while (0<x)\n"
                                   ""
                                << std::make_pair(1, 13) << "\nok"
                                << "int fla() {\n"
                                   "  while (0<x)\n"
                                   "    ok\n";

        QTest::newRow("while2") << "int fla() {\n"
                                   "  while (0<x)\n"
                                   "    x--;\n"
                                   ""
                                << std::make_pair(2, 8) << "\nok"
                                << "int fla() {\n"
                                   "  while (0<x)\n"
                                   "    x--;\n"
                                   "  ok\n";

        QTest::newRow("while3") << "int fla() {\n"
                                   "  while (0<x) x();\n"
                                   ""
                                << std::make_pair(1, 18) << "\nok"
                                << "int fla() {\n"
                                   "  while (0<x) x();\n"
                                   "  ok\n";
    }

    void For() { runDataDrivenTest(); }

    void For_data() {
        addColumns();
        QTest::newRow("for1") << "int main() {\n"
                                 "  for (int a = 0;\n"
                                 ""
                              << std::make_pair(1, 17) << "\nb"
                              << "int main() {\n"
                                 "  for (int a = 0;\n"
                                 "       b\n";
        QTest::newRow("for2") << "int main() {\n"
                                 "  for (int a = 0;\n"
                                 "       b;\n"
                                 "       c)\n"
                                 ""
                              << std::make_pair(3, 9) << " {\nok"
                              << "int main() {\n"
                                 "  for (int a = 0;\n"
                                 "       b;\n"
                                 "       c) {\n"
                                 "    ok\n";

        QTest::newRow("for3") << "int fla() {\n"
                                 "  for (;0<x;)\n"
                                 ""
                              << std::make_pair(1, 13) << "\nok"
                              << "int fla() {\n"
                                 "  for (;0<x;)\n"
                                 "    ok\n";

        QTest::newRow("for4") << "int fla() {\n"
                                 "  for (;0<x;)\n"
                                 "    x--;\n"
                                 ""
                              << std::make_pair(2, 8) << "\nok"
                              << "int fla() {\n"
                                 "  for (;0<x;)\n"
                                 "    x--;\n"
                                 "  ok\n";

        QTest::newRow("for5") << "int fla() {\n"
                                 "  for (;0<x;) x();\n"
                                 ""
                              << std::make_pair(1, 18) << "\nok"
                              << "int fla() {\n"
                                 "  for (;0<x;) x();\n"
                                 "  ok\n";
    }

    void Do() { runDataDrivenTest(); }

    void Do_data() {
        addColumns();

        QTest::newRow("do1") << "int fla() {\n"
                                "  do\n"
                                ""
                             << std::make_pair(1, 4) << "\nok"
                             << "int fla() {\n"
                                "  do\n"
                                "    ok\n";

        QTest::newRow("do2") << "int fla() {\n"
                                "  do\n"
                                "    x--;\n"
                                ""
                             << std::make_pair(2, 8) << "\nwhile"
                             << "int fla() {\n"
                                "  do\n"
                                "    x--;\n"
                                "  while\n";

        QTest::newRow("do3") << "int fla() {\n"
                                "  do x();\n"
                                ""
                             << std::make_pair(1, 9) << "\nwhile"
                             << "int fla() {\n"
                                "  do x();\n"
                                "  while\n";
    }

    void Switch() { runDataDrivenTest(); }

    void Switch_data() {
        addColumns();

        QTest::newRow("switch1") << "  int foo() {\n"
                                    "    switch (x) {\n"
                                    ""
                                 << std::make_pair(1, 16) << "\ncase 0:"
                                 << "  int foo() {\n"
                                    "    switch (x) {\n"
                                    "      case 0:\n";

        QTest::newRow("switch2") << "  int foo() {\n"
                                    "    switch (x) {\n"
                                    "      case 0:\n"
                                    ""
                                 << std::make_pair(2, 13) << "\nok"
                                 << "  int foo() {\n"
                                    "    switch (x) {\n"
                                    "      case 0:\n"
                                    "        ok\n";

        QTest::newRow("switch3") << "  int foo() {\n"
                                    "    switch (x) {\n"
                                    "      case 0:\n"
                                    "        ok;\n"
                                    ""
                                 << std::make_pair(3, 11) << "\ncase 1:"
                                 << "  int foo() {\n"
                                    "    switch (x) {\n"
                                    "      case 0:\n"
                                    "        ok;\n"
                                    "      case 1:\n";

        QTest::newRow("switch4") << "  int foo() {\n"
                                    "    switch (x) {\n"
                                    "      case 0:\n"
                                    "        ok;\n"
                                    "      case 1:\n"
                                    ""
                                 << std::make_pair(4, 13) << ";\n}"
                                 << "  int foo() {\n"
                                    "    switch (x) {\n"
                                    "      case 0:\n"
                                    "        ok;\n"
                                    "      case 1:;\n"
                                    "    }\n";

        QTest::newRow("switch5") << "  int foo() {\n"
                                    "    switch (x) {\n"
                                    "      case 0:\n"
                                    ""
                                 << std::make_pair(2, 13) << "\ncase 1:"
                                 << "  int foo() {\n"
                                    "    switch (x) {\n"
                                    "      case 0:\n"
                                    "      case 1:\n";

        QTest::newRow("switch6") << "  int foo() {\n"
                                    "    switch (x) {\n"
                                    "      case 0:\n"
                                    "      case 1:\n"
                                    ""
                                 << std::make_pair(3, 13) << " // bla"
                                 << "  int foo() {\n"
                                    "    switch (x) {\n"
                                    "      case 0:\n"
                                    "      case 1: // bla\n";

        QTest::newRow("switch7") << "  int foo() {\n"
                                    "    switch (x) {\n"
                                    "      case 0:\n"
                                    "        ok;\n"
                                    "      case 1:\n"
                                    ""
                                 << std::make_pair(4, 13) << "\ndefault:\n;"
                                 << "  int foo() {\n"
                                    "    switch (x) {\n"
                                    "      case 0:\n"
                                    "        ok;\n"
                                    "      case 1:\n"
                                    "      default:\n"
                                    "        ;\n";

#if 0 // FIXME probably requires understanding what is text and what is not
        QTest::newRow("switch8")
            <<  "  int foo() {\n"
                "    switch (x) {\n"
                "      case '.':\n"
                "        ok;\n"
                ""
            <<  std::make_pair(3, 11)
            <<  "\ncase ':'"
            <<  "  int foo() {\n"
                "    switch (x) {\n"
                "      case '.':\n"
                "        ok;\n"
                "        case ':'\n";
#endif

        QTest::newRow("switch9") << "  int foo() {\n"
                                    "    switch (x) {\n"
                                    "      case '.':\n"
                                    "        ok;\n"
                                    "        case ':'\n"
                                    ""
                                 << std::make_pair(4, 16) << ":"
                                 << "  int foo() {\n"
                                    "    switch (x) {\n"
                                    "      case '.':\n"
                                    "        ok;\n"
                                    "      case ':':\n";

#if 0 // FIXME  AK: I don't understand, why this tests shall pass. kate works
      // like qutepart
        QTest::newRow("switch10")
            <<  "  int foo() {\n"
                "    switch (x) { // only first symbolic colon may reindent\n"
                "    case '0':\n"
                ""
            <<  std::make_pair(2, 13)
            <<  " case '1':"
            <<  "  int foo() {\n"
                "    switch (x) { // only first symbolic colon may reindent\n"
                "    case '0': case '1':\n";

        QTest::newRow("switch11")
            <<  "  int foo() {\n"
                "    switch (x) { // only first symbolic colon may reindent\n"
                "    case '0': case '1':\n"
                ""
            <<  std::make_pair(2, 23)
            <<  " case '2':"
            <<  "  int foo() {\n"
                "    switch (x) { // only first symbolic colon may reindent\n"
                "    case '0': case '1': case '2':\n";
#endif

        QTest::newRow("switch12") << "int fla() {\n"
                                     "  switch (x)\n"
                                     ""
                                  << std::make_pair(1, 12) << "\nok"
                                  << "int fla() {\n"
                                     "  switch (x)\n"
                                     "    ok\n";

#if 0 // FIXME  AK: I don't understand, why this tests shall pass. kate works
      // like qutepart
        QTest::newRow("switch13")
            <<  "int fla() {\n"
                "  switch (x)\n"
                "    x--;\n"
                ""
            <<  std::make_pair(2, 8)
            <<  "\nok"
            <<  "int fla() {\n"
                "  switch (x)\n"
                "    x--;\n"
                "  ok\n";
#endif

        QTest::newRow("switch14") << "int fla() {\n"
                                     "  switch (x) x();\n"
                                     ""
                                  << std::make_pair(1, 17) << "\nok"
                                  << "int fla() {\n"
                                     "  switch (x) x();\n"
                                     "  ok\n";
    }

    void Visib() { runDataDrivenTest(); }

    void Visib_data() {
        addColumns();

        QTest::newRow("visib1") << "class A {\n"
                                   "  public:\n"
                                   ""
                                << std::make_pair(1, 9) << "\nA()"
                                << "class A {\n"
                                   "  public:\n"
                                   "    A()\n";

#if 0 // this indentation alogrithm disabled. Not indenting access modifiers
        QTest::newRow("visib2")
            <<  "class A {\n"
                "  public:\n"
                "    A();\n"
                ""
            <<  std::make_pair(2, 8)
            <<  "\nprotected:"
            <<  "class A {\n"
                "  public:\n"
                "    A();\n"
                "  protected:\n";

        QTest::newRow("visib3")
            <<  "class A {\n"
                "  public:\n"
                ""
            <<  std::make_pair(1, 9)
            <<  "\nprotected:"
            <<  "class A {\n"
                "  public:\n"
                "  protected:\n";
#endif
        QTest::newRow("visib4") << "class A {\n"
                                   "             public:\n"
                                   ""
                                << std::make_pair(1, 20) << " // :"
                                << "class A {\n"
                                   "             public: // :\n";

        QTest::newRow("visib5") << "class A {\n"
                                   "             public:\n"
                                   ""
                                << std::make_pair(1, 20) << " x(\":\");"
                                << "class A {\n"
                                   "             public: x(\":\");\n";

        QTest::newRow("visib6") << "class A {\n"
                                   "             public:\n"
                                   ""
                                << std::make_pair(1, 20) << " x(':');"
                                << "class A {\n"
                                   "             public: x(':');\n";

        QTest::newRow("visib7") << "class A {\n"
                                   "             public:\n"
                                   ""
                                << std::make_pair(1, 20) << " X::x();"
                                << "class A {\n"
                                   "             public: X::x();\n";

        QTest::newRow("visib8") << "class A {\n"
                                   "             public:\n"
                                   ""
                                << std::make_pair(1, 20) << " private:"
                                << "class A {\n"
                                   "             public: private:\n";
    }

    void Comment() { runDataDrivenTest(); }

    void Comment_data() {
        addColumns();

        QTest::newRow("comment1") << "  int foo() {\n"
                                     "    x;\n"
                                     "//     y;\n"
                                     ""
                                  << std::make_pair(2, 9) << "\nok"
                                  << "  int foo() {\n"
                                     "    x;\n"
                                     "//     y;\n"
                                     "    ok\n";

        QTest::newRow("comment2") << "foo(); // \"comment\"\n"
                                     ""
                                  << std::make_pair(0, 19) << "\nok"
                                  << "foo(); // \"comment\"\n"
                                     "ok\n";
    }

    void Aplist() { runDataDrivenTest(); }

    void Aplist_data() {
        addColumns();

        QTest::newRow("aplist1") << "int main(int argc, char **argv) {\n"
                                    "  somefunctioncall(argc,\n"
                                    ""
                                 << std::make_pair(1, 24) << "\nok"
                                 << "int main(int argc, char **argv) {\n"
                                    "  somefunctioncall(argc,\n"
                                    "                   ok\n";

        QTest::newRow("aplist2") << "int main(int argc, char **argv) {\n"
                                    "  somefunctioncall(argc,\n"
                                    "                   ok,\n"
                                    ""
                                 << std::make_pair(2, 22) << "\nargv[0]"
                                 << "int main(int argc, char **argv) {\n"
                                    "  somefunctioncall(argc,\n"
                                    "                   ok,\n"
                                    "                   argv[0]\n";

        QTest::newRow("aplist3") << "int main(int argc, char **argv) {\n"
                                    "  somefunctioncall(argc,\n"
                                    "                   ok,\n"
                                    "                   argv[0]\n"
                                    ""
                                 << std::make_pair(3, 26) << ");\nok"
                                 << "int main(int argc, char **argv) {\n"
                                    "  somefunctioncall(argc,\n"
                                    "                   ok,\n"
                                    "                   argv[0]);\n"
                                    "  ok\n";

        QTest::newRow("aplist4") << "int main(int argc, char **argv) {\n"
                                    "  somefunctioncall(argc,\n"
                                    "                   nestedcall(var,\n"
                                    ""
                                 << std::make_pair(2, 34) << "\nok"
                                 << "int main(int argc, char **argv) {\n"
                                    "  somefunctioncall(argc,\n"
                                    "                   nestedcall(var,\n"
                                    "                              ok\n";

        QTest::newRow("aplist5") << "int main(int argc, char **argv) {\n"
                                    "  f1(argc,\n"
                                    "     f2(var,\n"
                                    "        ok\n"
                                    ""
                                 << std::make_pair(3, 10) << "),\nargv"
                                 << "int main(int argc, char **argv) {\n"
                                    "  f1(argc,\n"
                                    "     f2(var,\n"
                                    "        ok),\n"
                                    "     argv\n";

        QTest::newRow("aplist6") << "int main(int argc, char **argv) {\n"
                                    "  f1(argc,\n"
                                    "     f2(var,\n"
                                    "        ok\n"
                                    ""
                                 << std::make_pair(3, 10) << "));\nok"
                                 << "int main(int argc, char **argv) {\n"
                                    "  f1(argc,\n"
                                    "     f2(var,\n"
                                    "        ok));\n"
                                    "  ok\n";

        QTest::newRow("aplist8") << "int main(int argc, char **argv) {\n"
                                    "  somefunctioncall(nestedcall(var,\n"
                                    ""
                                 << std::make_pair(1, 34) << "\nok"
                                 << "int main(int argc, char **argv) {\n"
                                    "  somefunctioncall(nestedcall(var,\n"
                                    "                              ok\n";

        QTest::newRow("aplist9") << "int main(int argc, char **argv) {\n"
                                    "  f1(f2(var,\n"
                                    "        ok\n"
                                    ""
                                 << std::make_pair(2, 10) << "),\nvar"
                                 << "int main(int argc, char **argv) {\n"
                                    "  f1(f2(var,\n"
                                    "        ok),\n"
                                    "     var\n";

        QTest::newRow("aplist10") << "int main(int argc, char **argv) {\n"
                                     "  somefunctioncall(\n"
                                     ""
                                  << std::make_pair(1, 19) << "\nok"
                                  << "int main(int argc, char **argv) {\n"
                                     "  somefunctioncall(\n"
                                     "    ok\n";

        QTest::newRow("aplist11") << "int main(int argc, char **argv) {\n"
                                     "  somefunctioncall(\n"
                                     "    ok\n"
                                     ""
                                  << std::make_pair(2, 6) << "\n)"
                                  << "int main(int argc, char **argv) {\n"
                                     "  somefunctioncall(\n"
                                     "    ok\n"
                                     "  )\n";

        QTest::newRow("aplist12") << "int main(int argc, char **argv) {\n"
                                     "  somefunctioncall(\n"
                                     "                   ok\n"
                                     "                  )\n"
                                     ""
                                  << std::make_pair(3, 19) << ";\nok"
                                  << "int main(int argc, char **argv) {\n"
                                     "  somefunctioncall(\n"
                                     "                   ok\n"
                                     "                  );\n"
                                     "  ok\n";

        QTest::newRow("aplist13") << "int main(int argc, char **argv) {\n"
                                     "  somefunctioncall(argc,argv,\n"
                                     "                   ok,\n"
                                     ""
                                  << std::make_pair(1, 24) << "\n"
                                  << "int main(int argc, char **argv) {\n"
                                     "  somefunctioncall(argc,\n"
                                     "                   argv,\n"
                                     "                   ok,\n";

        QTest::newRow("aplist14") << "int main(int argc, char **argv) {\n"
                                     "  somefunctioncall(argc, argv,\n"
                                     "                   ok,\n"
                                     ""
                                  << std::make_pair(1, 24) << "\n"
                                  << "int main(int argc, char **argv) {\n"
                                     "  somefunctioncall(argc,\n"
                                     "                   argv,\n"
                                     "                   ok,\n";

        QTest::newRow("aplist15") << "int main(int argc, char **argv) {\n"
                                     "  somefunctioncall(argc,\n"
                                     "                   argv,argx,\n"
                                     "                   ok,\n"
                                     ""
                                  << std::make_pair(2, 24) << "\n"
                                  << "int main(int argc, char **argv) {\n"
                                     "  somefunctioncall(argc,\n"
                                     "                   argv,\n"
                                     "                   argx,\n"
                                     "                   ok,\n";

        QTest::newRow("aplist16") << "int main(int argc, char **argv) {\n"
                                     "  somefunctioncall(argc,\n"
                                     "                   argv, argx,\n"
                                     "                   ok,\n"
                                     ""
                                  << std::make_pair(2, 24) << "\n"
                                  << "int main(int argc, char **argv) {\n"
                                     "  somefunctioncall(argc,\n"
                                     "                   argv,\n"
                                     "                   argx,\n"
                                     "                   ok,\n";

        QTest::newRow("aplist17") << "int main(int argc, char **argv) {\n"
                                     "  somefunctioncall(argc,\n"
                                     "                   nestedcall(var,argv,\n"
                                     ""
                                  << std::make_pair(2, 34) << "\n"
                                  << "int main(int argc, char **argv) {\n"
                                     "  somefunctioncall(argc,\n"
                                     "                   nestedcall(var,\n"
                                     "                              argv,\n";

        QTest::newRow("aplist18") << "int main(int argc, char **argv) {\n"
                                     "  somefunctioncall(argc,\n"
                                     "                   nestedcall(var, argv,\n"
                                     ""
                                  << std::make_pair(2, 34) << "\n"
                                  << "int main(int argc, char **argv) {\n"
                                     "  somefunctioncall(argc,\n"
                                     "                   nestedcall(var,\n"
                                     "                              argv,\n";

        QTest::newRow("aplist19") << "int main(int argc, char **argv) {\n"
                                     "  somefunctioncall(argc,\n"
                                     "                   nestedcall(var,\n"
                                     "                              argv,argx,\n"
                                     ""
                                  << std::make_pair(3, 35) << "\n"
                                  << "int main(int argc, char **argv) {\n"
                                     "  somefunctioncall(argc,\n"
                                     "                   nestedcall(var,\n"
                                     "                              argv,\n"
                                     "                              argx,\n";

        QTest::newRow("aplist20") << "int main(int argc, char **argv) {\n"
                                     "  somefunctioncall(argc,\n"
                                     "                   nestedcall(var,\n"
                                     "                              argv, argx,\n"
                                     ""
                                  << std::make_pair(3, 35) << "\n"
                                  << "int main(int argc, char **argv) {\n"
                                     "  somefunctioncall(argc,\n"
                                     "                   nestedcall(var,\n"
                                     "                              argv,\n"
                                     "                              argx,\n";

        QTest::newRow("aplist21") << "\t\t fu1(argc,\n"
                                     ""
                                  << std::make_pair(0, 12) << "\nargv"
                                  << "\t\t fu1(argc,\n"
                                     "         argv\n";
    }

    void OpenPar() { runDataDrivenTest(); }

    void OpenPar_data() {
        addColumns();

        QTest::newRow("openpar1") << "int main() {\n"
                                     ""
                                  << std::make_pair(0, 12) << "\nok"
                                  << "int main() {\n"
                                     "  ok\n";

        QTest::newRow("openpar2") << "int main()\n"
                                     ""
                                  << std::make_pair(0, 10) << "\n{\nok"
                                  << "int main()\n"
                                     "{\n"
                                     "  ok\n";

        QTest::newRow("openpar3") << "int main() {bla\n"
                                     ""
                                  << std::make_pair(0, 12) << "\n"
                                  << "int main() {\n"
                                     "  bla\n";

        QTest::newRow("openpar4") << "int main() {    bla\n"
                                     ""
                                  << std::make_pair(0, 12) << "\n"
                                  << "int main() {\n"
                                     "  bla\n";

        QTest::newRow("openpar5") << "int main() {foo();\n"
                                     ""
                                  << std::make_pair(0, 12) << "\n"
                                  << "int main() {\n"
                                     "  foo();\n";

        QTest::newRow("openpar6") << "int main()\n"
                                     "{bla\n"
                                  << std::make_pair(1, 1) << "\n"
                                  << "int main()\n"
                                     "{\n"
                                     "  bla\n";

        QTest::newRow("openpar7") << "int main()\n"
                                     "{    bla\n"
                                     ""
                                  << std::make_pair(1, 1) << "\n"
                                  << "int main()\n"
                                     "{\n"
                                     "  bla\n";

        QTest::newRow("openpar8") << "int main()\n"
                                     "{foo();\n"
                                     ""
                                  << std::make_pair(1, 1) << "\n"
                                  << "int main()\n"
                                     "{\n"
                                     "  foo();\n";

        QTest::newRow("openpar9") << "int main() {\n"
                                     "  if (x) {\n"
                                     "    a;\n"
                                     "  } else\n"
                                     ""
                                  << std::make_pair(3, 8) << " {\nok"
                                  << "int main() {\n"
                                     "  if (x) {\n"
                                     "    a;\n"
                                     "  } else {\n"
                                     "    ok\n";

        QTest::newRow("openpar10") << "int main() {\n"
                                      "  if (x) {\n"
                                      "    a;\n"
                                      "  } else if (y, z)\n"
                                      ""
                                   << std::make_pair(3, 18) << " {\nok"
                                   << "int main() {\n"
                                      "  if (x) {\n"
                                      "    a;\n"
                                      "  } else if (y, z) {\n"
                                      "    ok\n";
    }

    void ClosPar() { runDataDrivenTest(); }

    void ClosPar_data() {
        addColumns();

        QTest::newRow("clospar1") << "int main() {\n"
                                     "  ok;\n"
                                     ""
                                  << std::make_pair(1, 5) << "\n}"
                                  << "int main() {\n"
                                     "  ok;\n"
                                     "}\n";

        QTest::newRow("clospar2") << "int main()\n"
                                     "{\n"
                                     "  ok;\n"
                                     ""
                                  << std::make_pair(2, 5) << "\n}"
                                  << "int main()\n"
                                     "{\n"
                                     "  ok;\n"
                                     "}\n";

#if 0 // FIXME not supported by C++ version
        QTest::newRow("clospar3")
            <<  "int main() {\n"
                "  ok;}\n"
                ""
            <<  std::make_pair(1, 5)
            <<  "\n"
            <<  "int main() {\n"
                "  ok;\n"
                "  \n"
                "}\n";

        QTest::newRow("clospar4")
            <<  "int main() {\n"
                "  for() {\n"
                "    x;}\n"
                ""
            <<  std::make_pair(2, 6)
            <<  "\n"
            <<  "int main() {\n"
                "  for() {\n"
                "    x;\n"
                "    \n"
                "  }\n";
#endif
    }

    void PList() { runDataDrivenTest(); }

    void PList_data() {
        addColumns();

        QTest::newRow("plist1") << "int fla(int x,\n"
                                   ""
                                << std::make_pair(0, 14) << "\nshort u"
                                << "int fla(int x,\n"
                                   "        short u\n";

        QTest::newRow("plist2") << "int fla(int x,\n"
                                   "        short u\n"
                                   ""
                                << std::make_pair(1, 15) << ","
                                << "int fla(int x,\n"
                                   "        short u,\n";

        QTest::newRow("plist3") << "int fla(int x,\n"
                                   "        short u,\n"
                                   ""
                                << std::make_pair(1, 16) << "\nchar c)"
                                << "int fla(int x,\n"
                                   "        short u,\n"
                                   "        char c)\n";

        QTest::newRow("plist4") << "int fla(int x,\n"
                                   "        short u,\n"
                                   "        char c)\n"
                                   ""
                                << std::make_pair(2, 15) << "\n{"
                                << "int fla(int x,\n"
                                   "        short u,\n"
                                   "        char c)\n"
                                   "{\n";

        QTest::newRow("plist5") << "int fla(int x,\n"
                                   "        short u,\n"
                                   "        char c)\n"
                                   ""
                                << std::make_pair(2, 15) << " {\nok"
                                << "int fla(int x,\n"
                                   "        short u,\n"
                                   "        char c) {\n"
                                   "  ok\n";

        QTest::newRow("plist6") << "uint8_t func( uint8_t p1, uint8_t p2)\n"
                                   ""
                                << std::make_pair(0, 25) << "\n"
                                << "uint8_t func( uint8_t p1,\n"
                                   "              uint8_t p2)\n";

        QTest::newRow("plist7") << "\n"
                                   "uint8_t func( uint8_t p1, uint8_t p2)\n"
                                   ""
                                << std::make_pair(1, 25) << "\n"
                                << "\n"
                                   "uint8_t func( uint8_t p1,\n"
                                   "              uint8_t p2)\n";

        QTest::newRow("plist8") << "int fla(int x,short u,char c)\n"
                                   ""
                                << std::make_pair(0, 14) << "\n"
                                << "int fla(int x,\n"
                                   "        short u,char c)\n";

        QTest::newRow("plist9") << "int fla(int x,\n"
                                   "        short u,char c)\n"
                                   ""
                                << std::make_pair(1, 16) << "\n"
                                << "int fla(int x,\n"
                                   "        short u,\n"
                                   "        char c)\n";

#if 0 // FIXME  AK: I don't understand, why this tests shall pass. kate works
      // like qutepart
        QTest::newRow("plist10")
            <<  "int fla(int x,short u,char c)\n"
                ""
            <<  std::make_pair(0, 8)
            <<  "\n"
            <<  "int fla(\n"
                "        int x,short u,char c)\n";
#endif

        QTest::newRow("plist11") << "int fla(\n"
                                    "        int x,short u,char c)\n"
                                    ""
                                 << std::make_pair(1, 14) << "\n"
                                 << "int fla(\n"
                                    "        int x,\n"
                                    "        short u,char c)\n";

        QTest::newRow("plist12") << "int fla(\n"
                                    "        int x,\n"
                                    "        short u,char c)\n"
                                    ""
                                 << std::make_pair(2, 16) << "\n"
                                 << "int fla(\n"
                                    "        int x,\n"
                                    "        short u,\n"
                                    "        char c)\n";

        QTest::newRow("plist13") << "int fla(\n"
                                    "        int x,\n"
                                    "        short u,\n"
                                    "        char c)\n"
                                    ""
                                 << std::make_pair(3, 14) << "\n"
                                 << "int fla(\n"
                                    "        int x,\n"
                                    "        short u,\n"
                                    "        char c\n"
                                    "       )\n";

#if 0 // FIXME  AK: I don't understand, why this tests shall pass. kate works
      // like qutepart
        QTest::newRow("plist14")
            <<  "int b() {\n"
                "}\n"
                "int fla(int x,short u,char c)\n"
                ""
            <<  std::make_pair(2, 8)
            <<  "\n"
            <<  "int b() {\n"
                "}\n"
                "int fla(\n"
                "        int x,short u,char c)\n";
#endif

        QTest::newRow("plist15") << "int fla(\n"
                                    "        int x,short u,char c\n"
                                    "       )\n"
                                    ""
                                 << std::make_pair(1, 14) << "\n"
                                 << "int fla(\n"
                                    "        int x,\n"
                                    "        short u,char c\n"
                                    "       )\n";

        QTest::newRow("plist16") << "int fla(\n"
                                    "        int x,short long_var_name,char c)\n"
                                    ""
                                 << std::make_pair(1, 14) << "\n"
                                 << "int fla(\n"
                                    "        int x,\n"
                                    "        short long_var_name,char c)\n";

        QTest::newRow("plist17") << "int fla(\n"
                                    "        int x,short long_var_name,\n"
                                    "        char c)\n"
                                    ""
                                 << std::make_pair(1, 14) << "\n"
                                 << "int fla(\n"
                                    "        int x,\n"
                                    "        short long_var_name,\n"
                                    "        char c)\n";

        QTest::newRow("plist18") << "void flp() {\n"
                                    "}\n"
                                    "\n"
                                    "int fla(\n"
                                    "        int x,short long_var_name,\n"
                                    "        char c)\n"
                                    ""
                                 << std::make_pair(4, 14) << "\n"
                                 << "void flp() {\n"
                                    "}\n"
                                    "\n"
                                    "int fla(\n"
                                    "        int x,\n"
                                    "        short long_var_name,\n"
                                    "        char c)\n";

        QTest::newRow("plist19") << "int x() {\n"
                                    "}\n"
                                    "int fla(\n"
                                    "        int x,short u,char c\n"
                                    "       )\n"
                                    ""
                                 << std::make_pair(3, 14) << "\n"
                                 << "int x() {\n"
                                    "}\n"
                                    "int fla(\n"
                                    "        int x,\n"
                                    "        short u,char c\n"
                                    "       )\n";

        QTest::newRow("plist20") << "void x() {\n"
                                    "}\n"
                                    "int fla(\n"
                                    "        int x,\n"
                                    "        short u,\n"
                                    "        char c)\n"
                                    ""
                                 << std::make_pair(5, 14) << "\n"
                                 << "void x() {\n"
                                    "}\n"
                                    "int fla(\n"
                                    "        int x,\n"
                                    "        short u,\n"
                                    "        char c\n"
                                    "       )\n";

        QTest::newRow("plist21") << "int x() {\n"
                                    "}\n"
                                    "int fla(\n"
                                    "        int x,\n"
                                    "        short u,char c)\n"
                                    ""
                                 << std::make_pair(4, 16) << "\n"
                                 << "int x() {\n"
                                    "}\n"
                                    "int fla(\n"
                                    "        int x,\n"
                                    "        short u,\n"
                                    "        char c)\n";

        QTest::newRow("plist22") << "int b() {\n"
                                    "}\n"
                                    "int fla(\n"
                                    "        int x,short u,char c)\n"
                                    ""
                                 << std::make_pair(3, 14) << "\n"
                                 << "int b() {\n"
                                    "}\n"
                                    "int fla(\n"
                                    "        int x,\n"
                                    "        short u,char c)\n";

        QTest::newRow("plist24") << "int b() {\n"
                                    "}\n"
                                    "int flablabberwabber(\n"
                                    "                     int lonng,short lonngearr,char shrt)\n"
                                    ""
                                 << std::make_pair(3, 31) << "\n"
                                 << "int b() {\n"
                                    "}\n"
                                    "int flablabberwabber(\n"
                                    "                     int lonng,\n"
                                    "                     short lonngearr,char shrt)\n";

        QTest::newRow("plist25") << "int fla(\n"
                                    "  int x,\n"
                                    "  short u,\n"
                                    "  char c)\n"
                                    ""
                                 << std::make_pair(3, 8) << "\n"
                                 << "int fla(\n"
                                    "  int x,\n"
                                    "  short u,\n"
                                    "  char c\n"
                                    ")\n";
    }

    void Comma() { runDataDrivenTest(); }

    void Comma_data() {
        addColumns();

        QTest::newRow("comma1") << "int fla() {\n"
                                   "  double x,\n"
                                   ""
                                << std::make_pair(1, 11) << "\nok"
                                << "int fla() {\n"
                                   "  double x,\n"
                                   "  ok\n";

        QTest::newRow("comma2") << "int fla() {\n"
                                   "  double x,y;\n"
                                   ""
                                << std::make_pair(1, 11) << "\n"
                                << "int fla() {\n"
                                   "  double x,\n"
                                   "  y;\n";

        QTest::newRow("comma3") << "int fla() {\n"
                                   "  b = 1,\n"
                                   ""
                                << std::make_pair(1, 8) << "\nok"
                                << "int fla() {\n"
                                   "  b = 1,\n"
                                   "  ok\n";

        QTest::newRow("comma4") << "int fla() {\n"
                                   "  b = 1,c = 2;\n"
                                   ""
                                << std::make_pair(1, 8) << "\n"
                                << "int fla() {\n"
                                   "  b = 1,\n"
                                   "  c = 2;\n";

        QTest::newRow("comma5") << "double x,\n"
                                   ""
                                << std::make_pair(0, 9) << "\nok"
                                << "double x,\n"
                                   "ok\n";

        QTest::newRow("comma6") << "double x,y;\n"
                                   ""
                                << std::make_pair(0, 9) << "\n"
                                << "double x,\n"
                                   "y;\n";
    }

    void Normal() { runDataDrivenTest(); }

    void Normal_data() {
        addColumns();

        QTest::newRow("normal1") << "int main() {\n"
                                    "    bla;"
                                 << std::make_pair(1, 8) << "\nok;"
                                 << "int main() {\n"
                                    "    bla;\n"
                                    "    ok;";

        QTest::newRow("normal2") << "int main() {\n"
                                    "    bla;blu;"
                                 << std::make_pair(1, 8) << "\n"
                                 << "int main() {\n"
                                    "    bla;\n"
                                    "    blu;";

        QTest::newRow("normal3") << "int main() {\n"
                                    "    bla;  blu;\n"
                                    ""
                                 << std::make_pair(1, 8) << "\n"
                                 << "int main() {\n"
                                    "    bla;\n"
                                    "    blu;\n";
    }

    void Using() { runDataDrivenTest(); }

    void Using_data() {
        addColumns();

#if 0 // FIXME  AK: I don't understand, why this tests shall pass. kate works
      // like qutepart
        QTest::newRow("using1")
            <<  "using\n"
                ""
            <<  std::make_pair(0, 5)
            <<  "\nok"
            <<  "using\n"
                "  ok;\n";

        QTest::newRow("using2")
            <<  "using\n"
                "  std::vector;\n"
                ""
            <<  std::make_pair(1, 14)
            <<  "\nok"
            <<  "using\n"
                "  std::vector;\n"
                "ok\n";
#endif

        QTest::newRow("using3") << "using std::vector;\n"
                                   ""
                                << std::make_pair(0, 18) << "\nok"
                                << "using std::vector;\n"
                                   "ok\n";
    }

    void Doxygen() { runDataDrivenTest(); }

    void Doxygen_data() {
        addColumns();

        QTest::newRow("doxygen1") << "class A {\n"
                                     "  /**\n"
                                     ""
                                  << std::make_pair(1, 5) << "\nconstructor"
                                  << "class A {\n"
                                     "  /**\n"
                                     "   * constructor\n";

        QTest::newRow("doxygen2") << "class A {\n"
                                     "  /**\n"
                                     "   * constructor\n"
                                     ""
                                  << std::make_pair(2, 16) << "\n@param x foo"
                                  << "class A {\n"
                                     "  /**\n"
                                     "   * constructor\n"
                                     "   * @param x foo\n";

        QTest::newRow("doxygen3") << "class A {\n"
                                     "  /**\n"
                                     "   * constructor\n"
                                     "   * @param x foo\n"
                                     ""
                                  << std::make_pair(3, 17) << "\n/"
                                  << "class A {\n"
                                     "  /**\n"
                                     "   * constructor\n"
                                     "   * @param x foo\n"
                                     "   */\n";

        QTest::newRow("doxygen4") << "class A {\n"
                                     "  /**\n"
                                     "   * constructor\n"
                                     "   * @param x foo\n"
                                     "   */\n"
                                     ""
                                  << std::make_pair(4, 5) << "\nok"
                                  << "class A {\n"
                                     "  /**\n"
                                     "   * constructor\n"
                                     "   * @param x foo\n"
                                     "   */\n"
                                     "  ok\n";

        QTest::newRow("doxygen5") << "class A {\n"
                                     "  /**\n"
                                     ""
                                  << std::make_pair(1, 5) << " constructor */"
                                  << "class A {\n"
                                     "  /** constructor */\n";

        QTest::newRow("doxygen6") << "class A {\n"
                                     "  /** constructor */\n"
                                     ""
                                  << std::make_pair(1, 20) << "\nok"
                                  << "class A {\n"
                                     "  /** constructor */\n"
                                     "  ok\n";

        QTest::newRow("doxygen7") << "class A {\n"
                                     "  int foo(); /** unorthodox doxygen comment */\n"
                                     ""
                                  << std::make_pair(1, 46) << "\nok"
                                  << "class A {\n"
                                     "  int foo(); /** unorthodox doxygen comment */\n"
                                     "  ok\n";

        QTest::newRow("doxygen8") << "/** unorthodox doxygen comment */ a;\n"
                                     ""
                                  << std::make_pair(0, 36) << "\nok"
                                  << "/** unorthodox doxygen comment */ a;\n"
                                     "ok\n";
    }

    void Prep() { runDataDrivenTest(); }

    void Prep_data() {
        addColumns();

        QTest::newRow("prep1") << "  int foo() {\n"
                                  "    x;\n"
                                  ""
                               << std::make_pair(1, 6) << "\n#ifdef FLA"
                               << "  int foo() {\n"
                                  "    x;\n"
                                  "#ifdef FLA\n";

        QTest::newRow("prep2") << "  int foo() {\n"
                                  "    x;\n"
                                  "#ifdef FLA\n"
                                  ""
                               << std::make_pair(2, 10) << "\nok"
                               << "  int foo() {\n"
                                  "    x;\n"
                                  "#ifdef FLA\n"
                                  "    ok\n";

#if 0 // FIXME probably, old tests. Now preprocessor is indented
        QTest::newRow("prep3")
            <<  "  int foo() {\n"
                "    x;\n"
                ""
            <<  std::make_pair(1, 6)
            <<  "\n#region FLA"
            <<  "  int foo() {\n"
                "    x;\n"
                "    #region FLA\n";

        QTest::newRow("prep4")
            <<  "  int foo() {\n"
                "    x;\n"
                ""
            <<  std::make_pair(1, 6)
            <<  "\n#endregion FLA"
            <<  "  int foo() {\n"
                "    x;\n"
                "    #endregion FLA\n";
#endif

        QTest::newRow("prep5") << "  int foo() {\n"
                                  "    x;\n"
                                  "#endregion FLA\n"
                                  ""
                               << std::make_pair(2, 14) << " // n"
                               << "  int foo() {\n"
                                  "    x;\n"
                                  "#endregion FLA // n\n";

        QTest::newRow("prep6") << "  int foo() {\n"
                                  "    x;\n"
                                  "#endregion\n"
                                  ""
                               << std::make_pair(2, 10) << " daten"
                               << "  int foo() {\n"
                                  "    x;\n"
                                  "#endregion daten\n";
    }

    void Forein() { runDataDrivenTest(); }

    void Forein_data() {
        addColumns();

        QTest::newRow("foreign1") << "// indent-width is 2 but we want to "
                                     "maintain an indentation of 4\n"
                                     "int main() {\n"
                                     ""
                                  << std::make_pair(1, 12) << "\n  bla();"
                                  << "// indent-width is 2 but we want to "
                                     "maintain an indentation of 4\n"
                                     "int main() {\n"
                                     "    bla();\n";

        QTest::newRow("foreign2") << "// indent-width is 2 but we want to "
                                     "maintain an indentation of 4\n"
                                     "int main() {\n"
                                     "    bla;\n"
                                     ""
                                  << std::make_pair(2, 8) << "\nbli();"
                                  << "// indent-width is 2 but we want to "
                                     "maintain an indentation of 4\n"
                                     "int main() {\n"
                                     "    bla;\n"
                                     "    bli();\n";

        QTest::newRow("foreign3") << "int main() {\n"
                                     "// indent-width is 2 but we want to "
                                     "maintain an indentation of 4\n"
                                     ""
                                  << std::make_pair(0, 12) << "\n  ok"
                                  << "int main() {\n"
                                     "    ok\n"
                                     "// indent-width is 2 but we want to "
                                     "maintain an indentation of 4\n";
    }

    void Other() { runDataDrivenTest(); }

    void Other_data() {
        addColumns();

        QTest::newRow("alignbrace") << "  if ( true ) {\n"
                                       "    \n"
                                       "}\n"
                                       ""
                                    << std::make_pair(2, 0) << "<<alignLine(2)>>"
                                    << "  if ( true ) {\n"
                                       "    \n"
                                       "  }\n";

        QTest::newRow("137157") << "# 1\n"
                                   "do {\n"
                                   "}\n"
                                   " while (0);\n"
                                   " "
                                << std::make_pair(4, 1) << "\nok"
                                << "# 1\n"
                                   "do {\n"
                                   "}\n"
                                   " while (0);\n"
                                   " \n"
                                   " ok";
    }
};

QTEST_MAIN(Test)
#include "test_indenter_cstyle.moc"

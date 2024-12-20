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
    void test() {
        qpart.setIndentAlgorithm(Qutepart::INDENT_ALG_SCHEME);
        runDataDrivenTest();
    }

    void test_data() {
        addColumns();

        QTest::newRow("1") << "(myfunc a" << std::make_pair(0, 9) << "\nb"
                           << "(myfunc a\n"
                              "        b";

        QTest::newRow("2") << "(myfunc (if (a) a b)" << std::make_pair(0, 20) << "\nb"
                           << "(myfunc (if (a) a b)\n"
                              "        b";

        QTest::newRow("3") << "(myfunc a)" << std::make_pair(0, 10) << "\nb"
                           << "(myfunc a)\n"
                              "b";

        QTest::newRow("4") << "  (myfunc a)" << std::make_pair(0, 12) << "\nb"
                           << "  (myfunc a)\n"
                              "  b";

        QTest::newRow("5") << "    (define" << std::make_pair(0, 11) << "\nb"
                           << "    (define\n"
                              "     b";

        QTest::newRow("6") << "a\n"
                              "b"
                           << std::make_pair(1, 1) << "\nx"
                           << "a\n"
                              "b\n"
                              "x";

        QTest::newRow("7") << "   a\n"
                              ""
                           << std::make_pair(1, 0) << "\n\tx"
                           << "   a\n"
                              "\n"
                              "   x";

        QTest::newRow("8") << "(define myfunc" << std::make_pair(0, 14) << "\nx"
                           << "(define myfunc\n"
                              "  x";

        QTest::newRow("9") << "(let ((pi 3.14) (r 120))" << std::make_pair(0, 24) << "\nx"
                           << "(let ((pi 3.14) (r 120))\n"
                              "  x";
    }

    void autoindent() {
        qpart.setIndentAlgorithm(Qutepart::INDENT_ALG_SCHEME);
        runDataDrivenTest();
    }

    void autoindent_data() {
        addColumns();

        QTest::newRow("1") << "(define (fac n)\n"
                              "(if (zero? n)\n"
                              "1\n"
                              "(* n (fac (- n 1)))))"
                           << std::make_pair(0, 0) << "<<alignLine(*)>>"
                           << "(define (fac n)\n"
                              "  (if (zero? n)\n"
                              "      1\n"
                              "      (* n (fac (- n 1)))))";

        QTest::newRow("2") << "(let ((fnord 5)\n"
                              "(answer 42))\n"
                              "(frobnicate fnord answer))"
                           << std::make_pair(0, 0) << "<<alignLine(*)>>"
                           << "(let ((fnord 5)\n"
                              "      (answer 42))\n"
                              "  (frobnicate fnord answer))";

        QTest::newRow("3") << "(list (foo)\n"
                              "(bar)\n"
                              "(baz))"
                           << std::make_pair(0, 0) << "<<alignLine(*)>>"
                           << "(list (foo)\n"
                              "      (bar)\n"
                              "      (baz))";

        QTest::newRow("4") << "(list\n"
                              "(foo)\n"
                              "(bar)\n"
                              "(baz))"
                           << std::make_pair(0, 0) << "<<alignLine(*)>>"
                           << "(list\n"
                              " (foo)\n"
                              " (bar)\n"
                              " (baz))";

        QTest::newRow("5") << "(let ((pi 3.14)\n"
                              "(r 120))\n"
                              "(* pi r r))"
                           << std::make_pair(0, 0) << "<<alignLine(*)>>"
                           << "(let ((pi 3.14)\n"
                              "      (r 120))\n"
                              "  (* pi r r))";

        QTest::newRow("6") << "(cond\n"
                              "((good? x) (handle-good x))\n"
                              "((bad? x)  (handle-bad x))\n"
                              "((ugly? x) (handle-ugly x))\n"
                              "(else      (handle-default x)))"
                           << std::make_pair(0, 0) << "<<alignLine(*)>>"
                           << "(cond\n"
                              " ((good? x) (handle-good x))\n"
                              " ((bad? x)  (handle-bad x))\n"
                              " ((ugly? x) (handle-ugly x))\n"
                              " (else      (handle-default x)))";

        QTest::newRow("7") << "(cond\n"
                              "((good? x)\n"
                              "(handle-good x))\n"
                              "((bad? x)\n"
                              "(handle-bad (if (really-bad? x)\n"
                              "(really-bad->bad x)\n"
                              "x)))\n"
                              "((ugly? x)\n"
                              "(handle-ugly x))\n"
                              "(else\n"
                              "(handle-default x)))"
                           << std::make_pair(0, 0) << "<<alignLine(*)>>"
                           << "(cond\n"
                              " ((good? x)\n"
                              "  (handle-good x))\n"
                              " ((bad? x)\n"
                              "  (handle-bad (if (really-bad? x)\n"
                              "                  (really-bad->bad x)\n"
                              "                  x)))\n"
                              " ((ugly? x)\n"
                              "  (handle-ugly x))\n"
                              " (else\n"
                              "  (handle-default x)))";

        QTest::newRow("8") << "(module gauss mzscheme\n"
                              "\n"
                              "(define (sum-up-to n)\n"
                              "(/ (* n (+ n 1))\n"
                              "2))\n"
                              "\n"
                              "(define (sum-up-to n)\n"
                              "(/ (* n (+ n 1))\n"
                              "2))\n"
                              "\n"
                              ")"
                           << std::make_pair(0, 0) << "<<alignLine(*)>>"
                           << "(module gauss mzscheme\n"
                              "\n"
                              "(define (sum-up-to n)\n"
                              "  (/ (* n (+ n 1))\n"
                              "     2))\n"
                              "\n"
                              "(define (sum-up-to n)\n"
                              "  (/ (* n (+ n 1))\n"
                              "     2))\n"
                              "\n"
                              ")";
    }
};

QTEST_MAIN(Test)
#include "test_indenter_scheme.moc"

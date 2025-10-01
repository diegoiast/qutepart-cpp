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
    void lisp_data() {
        addColumns();
        QTest::newRow("three semicolons") << "      \n"
                                             "   asdf"
                                          << std::make_pair(0, 6) << ";;;"
                                          << ";;;\n"
                                             "   asdf";

        QTest::newRow("two semicolons") << "      \n"
                                           "   asdf"
                                        << std::make_pair(0, 6) << ";;"
                                        << "   ;;\n"
                                           "   asdf";
#if 0
        QTest::newRow("find brace")
            << "  (bla                   (x (y (z)))" << std::make_pair(0, 36) << "\n"
            << "  (bla                   (x (y (z)))\n"
               "    ";
#endif
        QTest::newRow("not found brace")
            << "  (bla                   (x (y (z))))" << std::make_pair(0, 37) << "\n"
            << "  (bla                   (x (y (z))))\n";
    }

    void lisp() {
        qpart.setIndentAlgorithm(Qutepart::INDENT_ALG_LISP);
        qpart.setIndentWidth(2);
        runDataDrivenTest();
    }
};

QTEST_MAIN(Test)
#include "test_indenter_lisp.moc"

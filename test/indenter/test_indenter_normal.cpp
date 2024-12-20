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

    void normal_data() {
        addColumns();
        QTest::newRow("normal 2") << "    bla bla" << std::make_pair(0, 11) << "\nok"
                                  << "    bla bla\n"
                                     "    ok";

#if 0 // Now empty lines are not removed
        QTest::newRow("empty line 3")
            << "totally empty line\n"
               "\n"
               "\n"
            << std::make_pair(1, 0)
            << "\n\nok"
            << "    totally empty line\n"
               "\n"
               "\n"
               "    ok\n";
#endif

        QTest::newRow("empty line 1") << "      totally empty line\n"
                                         "\n"
                                      << std::make_pair(1, 0) << "\n\tok"
                                      << "      totally empty line\n"
                                         "\n"
                                         "      ok\n";

        QTest::newRow("normal 3") << "    bla bla\n"
                                     "    blu blu\n"
                                  << std::make_pair(1, 11) << "\nok"
                                  << "    bla bla\n"
                                     "    blu blu\n"
                                     "    ok\n";

        QTest::newRow("cascade 1") << "bla bla\n"
                                      "    blu blu\n"
                                   << std::make_pair(1, 11) << "\nok"
                                   << "bla bla\n"
                                      "    blu blu\n"
                                      "    ok\n";

        QTest::newRow("empty line 2") << "    empty line padded with 4 spcs\n"
                                         "    \n"
                                      << std::make_pair(1, 4) << "\nok"
                                      << "    empty line padded with 4 spcs\n"
                                         "    \n"
                                         "    ok\n";

        QTest::newRow("midbreak") << "    bla bla    blu blu\n"
                                  << std::make_pair(0, 11) << "\n"
                                  << "    bla bla\n"
                                     "    blu blu\n";

        QTest::newRow("midbreak 2") << "    bla bla blu blu\n"
                                    << std::make_pair(0, 11) << "\n"
                                    << "    bla bla\n"
                                       "    blu blu\n";

        QTest::newRow("normal 1") << "bla bla\n"
                                  << std::make_pair(0, 7) << "\nok"
                                  << "bla bla\n"
                                     "ok\n";

        QTest::newRow("newline") << "    sadf\n"
                                    "\n"
                                 << std::make_pair(1, 0) << "\nok"
                                 << "    sadf\n"
                                    "\n"
                                    "ok\n";
    }

    void normal() {
        qpart.setIndentAlgorithm(Qutepart::INDENT_ALG_NORMAL);
        runDataDrivenTest();
    }

    void lua_data() {
        addColumns();
        QTest::newRow("lua") << "    bla bla\n"
                             << std::make_pair(0, 11) << "\nok"
                             << "    bla bla\n"
                                "    ok\n";
    }

    void lua() {
        // lua is not implemented. Check is not crashed at least
        qpart.setIndentAlgorithm(Qutepart::INDENT_ALG_NORMAL);
        runDataDrivenTest();
    }
};

QTEST_MAIN(Test)
#include "test_indenter_normal.moc"

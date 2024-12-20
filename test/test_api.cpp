/*
 * Copyright (C) 2018-2023 Andrei Kopats
 * Copyright (C) 2023-...  Diego Iastrubni <diegoiast@gmail.com>
 * SPDX-License-Identifier: MIT
 */

#include <QDebug>
#include <QObject>
#include <QTest>

#include "qutepart.h"

class Test : public QObject {
    Q_OBJECT

  private slots:
    void TextCursorPosition() {
        Qutepart::Qutepart qutepart;
        qutepart.setPlainText("one\ntwo\nthree\nfour");
        qutepart.goTo(2);
        QCOMPARE(qutepart.textCursorPosition(), Qutepart::TextCursorPosition(2, 0));

        qutepart.goTo({2, 1});
        QCOMPARE(qutepart.textCursorPosition(), Qutepart::TextCursorPosition(2, 1));
    }
};

QTEST_MAIN(Test)
#include "test_api.moc"

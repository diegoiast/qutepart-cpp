/*
 * Copyright (C) 2018-2023 Andrei Kopats
 * Copyright (C) 2023-...  Diego Iastrubni <diegoiast@gmail.com>
 * SPDX-License-Identifier: MIT
 */

#include <utility>

#include <QObject>

#include "qutepart.h"

typedef std::pair<int, int> CursorPos;

class BaseTest : public QObject {
    Q_OBJECT

  protected:
    Qutepart::Qutepart qpart;

    void addColumns();

    void setCursorPosition(int line, int col);
    void enter();
    void tab();
    void type(const QString &text);
    void verifyExpected(const QString &expected);
    void runDataDrivenTest();

    virtual void init();
  private slots:
    void initTestCase();

  private:
    void autoIndentBlock(int blockIndex);
};

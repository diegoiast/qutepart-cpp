/*
 * Copyright (C) 2018-2023 Andrei Kopats
 * Copyright (C) 2023-...  Diego Iastrubni <diegoiast@gmail.com>
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <QStringList>

#include "context.h"
#include "style.h"

namespace Qutepart {

class AbstractRule;

class MatchResult {
  public:
    MatchResult(int length, const QStringList &data, bool lineContinue,
                const ContextSwitcher &context, const Style &style);
    MatchResult();

    int length;
    QStringList data;
    bool lineContinue;
    ContextSwitcher nextContext;
    Style style;
};

} // namespace Qutepart

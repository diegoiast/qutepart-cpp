/*
 * Copyright (C) 2018-2023 Andrei Kopats
 * Copyright (C) 2023-...  Diego Iastrubni <diegoiast@gmail.com>
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <QIcon>
#include <QTextBlockUserData>

#include "context_stack.h"

namespace Qutepart {

class TextBlockUserData : public QTextBlockUserData {
  public:
    TextBlockUserData(const QString &textTypeMap, const ContextStack &contexts);
    QString textTypeMap;
    ContextStack contexts;
    int state = 0;

    struct {
        QString message;
    } metaData;
};

} // namespace Qutepart

/*
 * Copyright (C) 2018-2023 Andrei Kopats
 * Copyright (C) 2023-...  Diego Iastrubni <diegoiast@gmail.com>
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <QIcon>
#include <QStack>
#include <QTextBlockUserData>

#include "context_stack.h"
#include "qutepart/diagnostic.h"

namespace Qutepart {

class Language;

class TextBlockUserData : public QTextBlockUserData {
  public:
    TextBlockUserData(const QString &textTypeMap, const ContextStack &contexts);
    QString textTypeMap;
    QVector<Language *> languageMap;
    ContextStack contexts;
    int state = 0;

    struct {
        int level = 0;
        bool folded = false;
    } folding;
    QStack<QString> regions;

    /// Diagnostics attributed to a named owner - one per language server, linter
    /// or build step. Kept apart from `state` and its severity bits so that
    /// clearing one owner cannot disturb another, nor the direct setLineError()
    /// marks that are not owned at all.
    QVector<Diagnostic> diagnostics;

    struct {
        QString message;
    } metaData;
};

} // namespace Qutepart

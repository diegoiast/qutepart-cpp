/*
 * Copyright (C) 2018-2023 Andrei Kopats
 * Copyright (C) 2023-...  Diego Iastrubni <diegoiast@gmail.com>
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <QIcon>
#include <QStack>
#include <QMap>
#include <QTextBlockUserData>

#include "context_stack.h"

namespace Qutepart {

class Language;

class TextBlockUserData : public QTextBlockUserData {
  public:
    ~TextBlockUserData() override;
  public:
    TextBlockUserData(const QString &textTypeMap, const ContextStack &contexts);
    QString textTypeMap;
    QVector<QSharedPointer<Language>> languageMap;
    ContextStack contexts;
    int state = 0;

    struct {
        int level = 0;
        bool folded = false;
    } folding;
    QStack<QString> regions;
    uint32_t magic = 0x51555445;
    QMap<QString, QTextBlockUserData*> additionalData;

    struct {
        QString message;
    } metaData;
};

} // namespace Qutepart

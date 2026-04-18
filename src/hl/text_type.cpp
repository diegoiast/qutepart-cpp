/*
 * Copyright (C) 2018-2023 Andrei Kopats
 * Copyright (C) 2023-...  Diego Iastrubni <diegoiast@gmail.com>
 * SPDX-License-Identifier: MIT
 */

#include "text_block_user_data.h"

#include "text_type.h"

namespace Qutepart {

namespace {

static inline TextBlockUserData *getData(const QTextBlock &block) {
    return dynamic_cast<TextBlockUserData *>(block.userData());
}

QChar getTextType(const QTextBlock &block, int column) {
    TextBlockUserData *data = getData(block);
    if (data == nullptr) {
        return ' ';
    } else {
        // this may happen on empty files
        if (column < 0 || data->textTypeMap.size() <= column) {
            return ' ';
        }
        return data->textTypeMap[column];
    }
}

} // namespace

QString textTypeMap(const QTextBlock &block) {
    const TextBlockUserData *data = getData(block);
    if (data == nullptr) {
        return QString().fill(' ', block.text().length());
    }

    return data->textTypeMap;
}

bool isCode(const QTextBlock &block, int column) {
    return getTextType(block, column) == TextType::Code;
}

bool isComment(const QTextBlock &block, int column) {
    QChar type = getTextType(block, column);
    return type == TextType::Comment || type == TextType::CommentSpell ||
           type == TextType::BlockComment || type == TextType::BlockCommentSpell ||
           type == TextType::HereDoc || type == TextType::HereDocSpell;
}

bool isBlockComment(const QTextBlock &block, int column) {
    QChar type = getTextType(block, column);
    return type == TextType::BlockComment || type == TextType::BlockCommentSpell;
}

bool isHereDoc(const QTextBlock &block, int column) {
    QChar type = getTextType(block, column);
    return type == TextType::HereDoc || type == TextType::HereDocSpell;
}

bool isSpellCheckable(const QTextBlock &block, int column) {
    return getTextType(block, column).isUpper();
}

} // namespace Qutepart

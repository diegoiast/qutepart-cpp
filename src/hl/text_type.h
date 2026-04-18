/*
 * Copyright (C) 2018-2023 Andrei Kopats
 * Copyright (C) 2023-...  Diego Iastrubni <diegoiast@gmail.com>
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <QTextBlock>

namespace Qutepart {

namespace TextType {
constexpr char Code = ' ';
constexpr char Comment = 'c';
constexpr char CommentSpell = 'C';
constexpr char BlockComment = 'b';
constexpr char BlockCommentSpell = 'B';
constexpr char HereDoc = 'h';
constexpr char HereDocSpell = 'H';
constexpr char String = 's';
constexpr char StringSpell = 'S';
} // namespace TextType

// Check if text at given position is a code
bool isCode(const QTextBlock &block, int column);

// Check if text at given position is a comment. Including block comments and
// here documents
bool isComment(const QTextBlock &block, int column);

// Check if text at given position is a block comment
bool isBlockComment(const QTextBlock &block, int column);

// Check if text at given position is a here document
bool isHereDoc(const QTextBlock &block, int column);

// Check if text at given position should be spell-checked (per syntax definition)
bool isSpellCheckable(const QTextBlock &block, int column);

QString textTypeMap(const QTextBlock &block);

} // namespace Qutepart

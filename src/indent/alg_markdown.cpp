/*
 * Copyright (C) 2023-...  Diego Iastrubni <diegoiast@gmail.com>
 * SPDX-License-Identifier: MIT
 */

#include <QRegularExpression>

#include "indent_funcs.h"

#include "alg_markdown.h"

namespace Qutepart {

QString IndentAlgMarkdown::computeSmartIndent(QTextBlock block, int /*cursorPos*/) const {
    // Unordered list: `  - item`, `  * item`, `  + item`
    auto static const rxUnordered = QRegularExpression (R"(^(\s*)([-*+])\s+)");
    // Task list: `- [ ] item` or `- [x] item`
    auto static const rxTask = QRegularExpression (R"(^\[[ xX]\]\s+)");
    // Ordered list: `  1. item`
    auto  static const rxOrdered = QRegularExpression (R"(^(\s*)(\d+)\.\s+)");
    // Blockquote: `> text` or nested `>> text`
    auto static const rxBlockquote = QRegularExpression (R"(^((?:>\s*)+))");
    // Headers (`# Heading`) — don't propagate the `#` prefix to the next line
    auto static const rxHeader = QRegularExpression (R"(^#{1,6}\s)");


    auto prevBlock = block.previous();
    if (!prevBlock.isValid()) {
        return QString();
    }

    auto prevText = prevBlock.text();
    auto m = rxUnordered.match(prevText);
    if (m.hasMatch()) {
        auto indent = m.captured(1);
        auto marker = m.captured(2);
        auto afterMarker = prevText.mid(m.capturedLength()).trimmed();

        if (afterMarker.isEmpty()) {
            return indent;
        }
        if (rxTask.match(afterMarker).hasMatch()) {
            return indent + marker + " [ ] ";
        }
        return indent + marker + " ";
    }

    auto m2 = rxOrdered.match(prevText);
    if (m2.hasMatch()) {
        auto indent = m2.captured(1);
        auto num = m2.captured(2).toInt();
        auto afterMarker = prevText.mid(m2.capturedLength()).trimmed();
        if (afterMarker.isEmpty()) {
            return indent;
        }
        return indent + QString::number(num + 1) + ". ";
    }

    auto m3 = rxBlockquote.match(prevText);
    if (m3.hasMatch()) {
        auto afterQuote = prevText.mid(m3.capturedLength()).trimmed();
        if (afterQuote.isEmpty()) {
            return QString();
        }
        auto depth = m3.captured(1).count('>');
        return QString(depth, '>') + ' ';
    }

    if (rxHeader.match(prevText).hasMatch()) {
        return QString();
    }

    return lineIndent(prevText);
}

} // namespace Qutepart

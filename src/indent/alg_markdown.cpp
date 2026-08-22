/*
 * Copyright (C) 2023-...  Diego Iastrubni <diegoiast@gmail.com>
 * SPDX-License-Identifier: MIT
 */

#include <QRegularExpression>

#include "indent_funcs.h"

#include "alg_markdown.h"

namespace Qutepart {

namespace {

struct LinePrefix {
    enum class Kind { Plain, Unordered, Ordered, Blockquote, Header };

    Kind kind = Kind::Plain;
    QString indent;     // leading whitespace of the line
    QString marker;     // list bullet: `-`, `*` or `+`
    QString prefix;     // indent + marker + the whitespace after the marker
    QString checkbox;   // `[ ] ` of a task item, including the whitespace after it
    QString rest;       // trimmed text after the prefix, checkbox included
    int number = 0;     // number of an ordered item
    int quoteDepth = 0; // amount of `>` of a blockquote
};

auto parseLinePrefix(const QString &text) -> LinePrefix {
    // Unordered list: `  - item`, `  * item`, `  + item`
    auto static const rxUnordered = QRegularExpression(R"(^(\s*)([-*+])\s+)");
    // Task list: `- [ ] item` or `- [x] item`
    auto static const rxTask = QRegularExpression(R"(^\[[ xX]\]\s+)");
    // Ordered list: `  1. item`
    auto static const rxOrdered = QRegularExpression(R"(^(\s*)(\d+)\.\s+)");
    // Blockquote: `> text` or nested `>> text`
    auto static const rxBlockquote = QRegularExpression(R"(^((?:>\s*)+))");
    // Headers (`# Heading`) — don't propagate the `#` prefix to the next line
    auto static const rxHeader = QRegularExpression(R"(^#{1,6}\s)");

    auto result = LinePrefix();

    auto m = rxUnordered.match(text);
    if (m.hasMatch()) {
        result.kind = LinePrefix::Kind::Unordered;
        result.indent = m.captured(1);
        result.marker = m.captured(2);
        result.prefix = m.captured(0);
        result.rest = text.mid(m.capturedLength()).trimmed();

        auto task = rxTask.match(result.rest);
        if (task.hasMatch()) {
            result.checkbox = task.captured(0);
        }
        return result;
    }

    auto m2 = rxOrdered.match(text);
    if (m2.hasMatch()) {
        result.kind = LinePrefix::Kind::Ordered;
        result.indent = m2.captured(1);
        result.number = m2.captured(2).toInt();
        result.prefix = m2.captured(0);
        result.rest = text.mid(m2.capturedLength()).trimmed();
        return result;
    }

    auto m3 = rxBlockquote.match(text);
    if (m3.hasMatch()) {
        result.kind = LinePrefix::Kind::Blockquote;
        result.prefix = m3.captured(0);
        result.quoteDepth = m3.captured(1).count('>');
        result.rest = text.mid(m3.capturedLength()).trimmed();
        return result;
    }

    if (rxHeader.match(text).hasMatch()) {
        result.kind = LinePrefix::Kind::Header;
        return result;
    }

    return result;
}

/* Amount of lines scanned backwards, looking for the marker of a wrapped item */
const int MAX_CONTINUATION_LINES = 100;

/* Whitespace as wide as `prefix`, keeping the tabs used for indentation */
auto blankPrefix(const QString &prefix) -> QString {
    auto result = prefix;
    for (auto &c : result) {
        if (c != '\t') {
            c = ' ';
        }
    }
    return result;
}

/* The list item a plain `block` continues, if any.
 * Lines wrapped out of an item are aligned with its text and carry no marker
 * of their own, the marker has to be looked up a few lines above.
 * Returns a Plain prefix when the line continues nothing.
 */
auto enclosingListItem(QTextBlock block) -> LinePrefix {
    auto continuation = lineIndent(block.text());
    if (continuation.isEmpty()) {
        return LinePrefix();
    }

    auto scanned = 0;
    for (auto b = block.previous(); b.isValid() && scanned < MAX_CONTINUATION_LINES;
         b = b.previous(), scanned++) {
        auto text = b.text();
        if (text.trimmed().isEmpty()) {
            // an empty line ends the item
            break;
        }

        auto prefix = parseLinePrefix(text);
        switch (prefix.kind) {
        case LinePrefix::Kind::Unordered:
        case LinePrefix::Kind::Ordered:
            // the item owns the line only if its text starts at (or before)
            // the column the continuation is aligned to
            if (blankPrefix(prefix.prefix + prefix.checkbox).length() <= continuation.length()) {
                return prefix;
            }
            return LinePrefix();

        case LinePrefix::Kind::Plain:
            // another continuation line of the same item
            if (lineIndent(text) != continuation) {
                return LinePrefix();
            }
            break;

        case LinePrefix::Kind::Blockquote:
        case LinePrefix::Kind::Header:
            return LinePrefix();
        }
    }

    return LinePrefix();
}

} // namespace

QString IndentAlgMarkdown::computeSmartIndent(QTextBlock block, int /*cursorPos*/) const {
    auto prevBlock = block.previous();
    if (!prevBlock.isValid()) {
        return QString();
    }

    auto prevText = prevBlock.text();
    auto prev = parseLinePrefix(prevText);
    if (prev.kind == LinePrefix::Kind::Plain) {
        // The previous line may be a wrapped continuation of a list item.
        // The list has to continue, even if the marker is a few lines above.
        auto item = enclosingListItem(prevBlock);
        if (item.kind != LinePrefix::Kind::Plain) {
            prev = item;
        }
    }

    switch (prev.kind) {
    case LinePrefix::Kind::Unordered:
        if (prev.rest.isEmpty()) {
            return prev.indent;
        }
        if (!prev.checkbox.isEmpty()) {
            return prev.indent + prev.marker + " [ ] ";
        }
        return prev.indent + prev.marker + " ";

    case LinePrefix::Kind::Ordered:
        if (prev.rest.isEmpty()) {
            return prev.indent;
        }
        return prev.indent + QString::number(prev.number + 1) + ". ";

    case LinePrefix::Kind::Blockquote:
        if (prev.rest.isEmpty()) {
            return QString();
        }
        return QString(prev.quoteDepth, '>') + ' ';

    case LinePrefix::Kind::Header:
        return QString();

    case LinePrefix::Kind::Plain:
        break;
    }

    return lineIndent(prevText);
}

QString IndentAlgMarkdown::computeWrapIndent(QTextBlock block, int /*cursorPos*/) const {
    auto prevBlock = block.previous();
    if (!prevBlock.isValid()) {
        return QString();
    }

    auto prevText = prevBlock.text();
    auto prev = parseLinePrefix(prevText);
    switch (prev.kind) {
    case LinePrefix::Kind::Unordered:
    case LinePrefix::Kind::Ordered:
        // The line continues the item, it does not start a new one. Repeating
        // the marker would, so align with the text of the current item instead.
        return blankPrefix(prev.prefix + prev.checkbox);

    case LinePrefix::Kind::Blockquote:
        return QString(prev.quoteDepth, '>') + ' ';

    case LinePrefix::Kind::Header:
    case LinePrefix::Kind::Plain:
        // A wrapped header is not a header anymore, keep the plain indentation.
        // Continuation lines are plain, so their own indent is reused.
        break;
    }

    return lineIndent(prevText);
}

} // namespace Qutepart

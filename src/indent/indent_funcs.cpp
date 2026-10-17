/*
 * Copyright (C) 2018-2023 Andrei Kopats
 * Copyright (C) 2023-...  Diego Iastrubni <diegoiast@gmail.com>
 * SPDX-License-Identifier: MIT
 */

#include "hl/text_type.h"
#include "text_block_utils.h"

#include "indent_funcs.h"

namespace Qutepart {

QString lineIndent(const QString &line) { return line.left(firstNonSpaceColumn(line)); }

QString blockIndent(QTextBlock block) { return lineIndent(block.text()); }

void setBlockIndent(QTextCursor *cursor, const QString &indent) {
    int currentIndentLength = blockIndent(cursor->block()).size();
    setPositionInBlock(cursor, 0, QTextCursor::MoveAnchor);
    setPositionInBlock(cursor, currentIndentLength, QTextCursor::KeepAnchor);
    cursor->insertText(indent);
}

QString prevBlockIndent(QTextBlock block) {
    QTextBlock prevBlock = block.previous();

    if (!block.isValid()) {
        return QString();
    }

    return blockIndent(prevBlock);
}

QString increaseIndent(const QString &line, const QString &indent) { return indent + line; }

QString decreaseIndent(const QString &line, const QString &indent) {
    if (line.startsWith(indent)) {
        return line.mid(indent.length());
    } else { // oops, strange indentation, just return previous indent
        return line;
    }
}

QString makeIndent(int confWidth, bool confUseTabs) {
    if (confUseTabs) {
        return QString("\t");
    } else {
        return QString().fill(' ', confWidth);
    }
}

/* Make indent text with specified with.
 * Contains width count of spaces, or tabs and spaces
 */
QString makeIndentFromWidth(int width, int confWidth, bool confUseTabs) {
    if (confUseTabs) {
        int tabCount = width / confWidth;
        int spaceCount = width % confWidth;
        return QString().fill('\t', tabCount) + QString().fill(' ', spaceCount);
    } else {
        return QString().fill(' ', width);
    }
}

QString makeIndentAsColumn(QTextBlock block, int column, int confIndentWidth, bool confUseTabs,
                           int offset) {
    QString blockText = block.text();
    QString textBeforeColumn = blockText.left(column);
    int tabCount = textBeforeColumn.count('\t');

    int visibleColumn = column + (tabCount * (confIndentWidth - 1));
    return makeIndentFromWidth(visibleColumn + offset, confIndentWidth, confUseTabs);
}

QString prevNonEmptyBlockIndent(const QTextBlock &block) {
    return blockIndent(prevNonEmptyBlock(block));
}

auto lineCommentContinuation(const QTextBlock &block, const QString &marker) -> QString {
    if (!block.isValid() || marker.isEmpty()) {
        return QString();
    }

    auto const text = block.text();
    auto const typeMap = textTypeMap(block);

    // Only a line which is a comment on its own is continued. A comment
    // behind code ("foo(); // bar") is not - it is a trailing comment.
    auto start = firstNonSpaceColumn(text);
    if (text.mid(start, marker.length()) != marker) {
        return QString();
    }

    /* The highlighter marks comment markers as comment. This rejects a line
    where the marker is part of a string, like '"// not a comment"'. */
    if ((start >= typeMap.length()) || (typeMap[start] != 'c')) {
        return QString();
    }

    // Keep the white space after the marker, so the new line is aligned with
    // the previous one. A comment without any text gets a single space - but
    // only one, repeating enter must not grow it.
    auto end = start + marker.length();
    while ((end < text.length()) && (text[end].isSpace())) {
        end++;
    }

    auto prefix = text.mid(start, end - start);
    if ((end >= text.length()) && (end == start + marker.length())) {
        prefix += ' ';
    }

    return lineIndent(text) + prefix;
}

QString textWithCommentsWiped(const QTextBlock &block) {
    QString text = block.text();
    QString typeMap = textTypeMap(block);
    for (int i = 0; i < text.length(); i++) {
        if (typeMap[i] == 'c') {
            text[i] = ' ';
        }
    }

    return text;
}

QChar firstNonSpaceChar(const QTextBlock &block) {
    QString textStripped = stripLeftWhitespace(block.text());
    if (textStripped.isEmpty()) {
        return QChar();
    } else {
        return textStripped[0];
    }
}

QChar lastNonSpaceChar(const QTextBlock &block) {
    QString textStripped = stripRightWhitespace(block.text());
    if (textStripped.isEmpty()) {
        return QChar();
    } else {
        return textStripped[textStripped.length() - 1];
    }
}

} // namespace Qutepart

/*
 * Copyright (C) 2018-2023 Andrei Kopats
 * Copyright (C) 2023-...  Diego Iastrubni <diegoiast@gmail.com>
 * SPDX-License-Identifier: MIT
 */

/* Lines API implementation.
The declarations (header) are in qutepart.h
*/

#include "qutepart.h"

namespace Qutepart {

Line::Line(const QTextBlock &block) : block_(block) {}

QString Line::text() const { return block_.text(); }

int Line::length() const { return block_.length() - 1; }

void Line::remove(int pos, int count) {
    auto blockLen = block_.length();

    if (pos < 0 || pos > blockLen) {
        qFatal("Wrong Line::remove(pos) %d", pos);
    }

    if (count < 0 || pos + count > blockLen) {
        qFatal("Wrong Line::remove(count) %d", count);
    }

    auto cursor = QTextCursor(block_);
    cursor.setPosition(block_.position() + pos);
    cursor.setPosition(block_.position() + pos + count, QTextCursor::KeepAnchor);
    cursor.removeSelectedText();
}

int Line::lineNumber() const { return block_.blockNumber(); }

LineIterator::LineIterator(const QTextBlock &block) : block_(block) {}

bool LineIterator::operator!=(const LineIterator &other) {
    if (block_.isValid()) {
        return block_ != other.block_;
    } else {
        return other.block_.isValid();
    }
}

bool LineIterator::operator==(const LineIterator &other) {
    if (block_.isValid()) {
        return block_ == other.block_;
    } else {
        return (!other.block_.isValid());
    }
}

LineIterator LineIterator::operator++() {
    LineIterator retVal = *this;
    block_ = block_.next();
    return retVal;
}

Line LineIterator::operator*() { return Line(block_); }

Lines::Lines(QTextDocument *document) : document_(document) {}

int Lines::count() const { return document_->blockCount(); }

Line Lines::at(int index) const { // Line count in the document
    return Line(document_->findBlockByNumber(index));
}

LineIterator Lines::begin() { return LineIterator(document_->firstBlock()); }

LineIterator Lines::end() { return LineIterator(QTextBlock()); }

Line Lines::first() const { return Line(document_->firstBlock()); }

Line Lines::last() const { return Line(document_->lastBlock()); }

void Lines::append(const QString &lineText) {
    QTextCursor cursor(document_->lastBlock());
    cursor.movePosition(QTextCursor::End);
    cursor.beginEditBlock();
    cursor.insertBlock();
    cursor.insertText(lineText);
    cursor.endEditBlock();
}

QString Lines::popAt(int lineNumber) {
    auto cursor = QTextCursor(document_->findBlockByNumber(lineNumber));
    auto result = cursor.block().text();
    auto removedEolAtStart = cursor.selectedText().startsWith(QChar(0x2029));

    cursor.beginEditBlock();
    cursor.select(QTextCursor::BlockUnderCursor);
    cursor.removeSelectedText();
    if (cursor.atStart()) {
        // remove \n after the line
        cursor.deleteChar();
    } else if (not removedEolAtStart) {
        // remove \n before the line
        cursor.deletePreviousChar();
    }
    cursor.endEditBlock();

    return result;
}

void Lines::insertAt(int lineNumber, const QString &text) {
    if (lineNumber < document_->blockCount()) {
        auto cursor = QTextCursor(document_->findBlockByNumber(lineNumber));
        cursor.beginEditBlock();
        cursor.insertText(text);
        cursor.insertBlock();
        cursor.endEditBlock();
    } else if (lineNumber == document_->blockCount()) {
        auto cursor = QTextCursor(document_);
        cursor.movePosition(QTextCursor::End);
        cursor.beginEditBlock();
        cursor.insertBlock();
        cursor.insertText(text);
        cursor.endEditBlock();
    } else {
        qFatal("Wrong line number %d at Lines::insertAt(). Have only %d", lineNumber,
               document_->blockCount());
    }
}

} // namespace Qutepart

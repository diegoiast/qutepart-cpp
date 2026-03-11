/*
 * Copyright (C) 2018-2023 Andrei Kopats
 * Copyright (C) 2023-...  Diego Iastrubni <diegoiast@gmail.com>
 * SPDX-License-Identifier: MIT
 */

#include <QDebug>
#include <QRegularExpression>

#include "hl/text_type.h"
#include "indent_funcs.h"
#include "text_block_utils.h"

#include "alg_ruby.h"

namespace Qutepart {

namespace {

// Unindent lines that match this regexp
const QRegularExpression rxUnindent("^\\s*((end|when|else|elsif|rescue|ensure)\\b|[\\]\\}\\)])(.*)$");

// Indent after lines that match this regexp
const QRegularExpression rxIndent("^\\s*([^=]*=[^=]\\s*)?(def|if|unless|for|while|until|class|module|else|elsif|"
                                  "case|when|begin|rescue|ensure|catch)\\b");

const QRegularExpression rxBlockEnd("\\s*end$");

bool isBlockContinuing(QTextBlock block) { return block.text().endsWith('\\'); }

int nextNonSpaceColumn(QTextBlock block, int column) {
    QString text = block.text();
    for (int i = column; i < text.length(); i++) {
        if (!text[i].isSpace()) {
            return i;
        }
    }
    return -1;
}

} // namespace

RubyStatement::RubyStatement(QTextBlock start, QTextBlock end) : startBlock(start), endBlock(end) {}

QString RubyStatement::toString() const {
    return QString("{ %1, %2}").arg(startBlock.blockNumber()).arg(endBlock.blockNumber());
}

// Return true if the given offset is a comment
bool RubyStatement::isPosCode(int column) const {
    if (column < 0) {
        return false;
    }
    return isCode(startBlock, column);
}

// Return the indent at the beginning of the statement
QString RubyStatement::indent() const { return blockIndent(startBlock); }

// Return the content of the statement from the document
QString RubyStatement::content() const {
    if (!contentCache_.isNull()) {
        return contentCache_;
    }

    QString cnt;

    QTextBlock block = startBlock;

    while (block != endBlock.next()) {
        QString text = textWithCommentsWiped(block);
        if (text.endsWith('\\')) {
            cnt += text.left(text.length() - 1);
            cnt += ' ';
        } else {
            cnt += text;
        }
        block = block.next();
    }

    contentCache_ = cnt;
    return cnt;
}

const QString &IndentAlgRuby::triggerCharacters() const {
    static QString chars = "cdefhilnrsuw})]";
    return chars;
}

bool IndentAlgRuby::isCommentBlock(QTextBlock block) const {
    QString text(block.text());
    int firstColumn = firstNonSpaceColumn(text);
    return (firstColumn == -1) || isComment(block, firstColumn);
}

/* Return the closest non-empty line, ignoring comments
(result <= line). Return -1 if the document
*/
QTextBlock IndentAlgRuby::prevNonCommentBlock(QTextBlock block) const {
    block = prevNonEmptyBlock(block);
    while (block.isValid() && isCommentBlock(block)) {
        block = prevNonEmptyBlock(block);
    }
    return block;
}

/* Return true if the given column is at least equal to the column that
contains the last non-whitespace character at the given line, or if
the rest of the line is a comment.
*/
bool IndentAlgRuby::isLastCodeColumn(QTextBlock block, int column) const {
    return column >= lastNonSpaceColumn(block.text()) ||
           isComment(block, nextNonSpaceColumn(block, column + 1));
}

bool IndentAlgRuby::isStmtContinuing(QTextBlock block) const {
    QString text = textWithCommentsWiped(block);

    TextPosition bracketPos = findAnyOpeningBracketBackward(TextPosition(block, block.length()));
    if (bracketPos.isValid()) {
        QChar bracket = bracketPos.block.text()[bracketPos.column];
        if (bracket == '(' || bracket == '[' || bracket == '<' ||
            (bracket == '{' && (!text.trimmed().endsWith('{')))) {
            return true;
        }
    }

    QRegularExpression rx("(\\+|\\-|\\*|\\/|\\=|&&|\\|\\||\\band\\b|\\bor\\b|,)\\s*$");
    QRegularExpressionMatch match = rx.match(text);

    return match.hasMatch() && isCode(block, match.capturedStart());
}

/* Return the first line that is not preceded by a "continuing" line.
Return currBlock if currBlock <= 0
*/
QTextBlock IndentAlgRuby::findStmtStart(QTextBlock block) const {
    QTextBlock prevBlock = prevNonCommentBlock(block);
    while (prevBlock.isValid() && isStmtContinuing(prevBlock)) {
        block = prevBlock;
        prevBlock = prevNonCommentBlock(block);
    }
    return block;
}

RubyStatement IndentAlgRuby::findPrevStmt(QTextBlock block) const {
    QTextBlock stmtEnd = prevNonCommentBlock(block);
    QTextBlock stmtStart = findStmtStart(stmtEnd);
    return RubyStatement(stmtStart, stmtEnd);
}

bool IndentAlgRuby::isBlockStart(const RubyStatement &stmt) const {
    QString content = stmt.content();
    QRegularExpressionMatch match = rxIndent.match(content);
    if (match.hasMatch()) {
        if (stmt.isPosCode(match.capturedStart(2))) {
            return true;
        }
    }

    QRegularExpression rx("((\\bdo\\b|\\{|\\(|\\[|<)(\\s*\\|.*\\|)?\\s*)$");
    QRegularExpressionMatch match2 = rx.match(content);
    if (match2.hasMatch()) {
        if (stmt.isPosCode(match2.capturedStart(2))) {
            return true;
        }
    }

    return false;
}

bool IndentAlgRuby::isBlockEnd(const RubyStatement &stmt) const {
    return rxUnindent.match(stmt.content()).hasMatch();
}

RubyStatement IndentAlgRuby::findBlockStart(QTextBlock block) const {
    RubyStatement currentStmt(block, block);
    int nested = isBlockEnd(currentStmt) ? 1 : 0;
    RubyStatement stmt = currentStmt;
    while (true) {
        if (!stmt.startBlock.isValid()) {
            return stmt;
        }

        stmt = findPrevStmt(stmt.startBlock);
        if (isBlockEnd(stmt)) {
            nested += 1;
        }

        if (isBlockStart(stmt)) {
            if (nested == 0) {
                return stmt;
            } else {
                nested -= 1;
                if (nested == 0) {
                    return stmt;
                }
            }
        }
    }
}

QString IndentAlgRuby::computeSmartIndent(QTextBlock block, int /*cursorPos*/) const {
    if (!isValidTrigger(block)) {
        return QString();
    }

    RubyStatement prevStmt = findPrevStmt(block);
    if (!prevStmt.endBlock.isValid()) {
        return QString(); // Can't indent the first line
    }

    QTextBlock prevBlock = prevNonEmptyBlock(block);

    // HACK Detect here documents
    if (isHereDoc(prevBlock, prevBlock.length() - 2)) {
        return QString();
    }

    // HACK Detect embedded comments
    if (isBlockComment(prevBlock, prevBlock.length() - 2)) {
        return QString();
    }

    QString prevStmtContent = prevStmt.content();
    QString prevStmtIndent = prevStmt.indent();

    if (rxUnindent.match(block.text()).hasMatch()) {
        RubyStatement startStmt = findBlockStart(block);
        if (startStmt.startBlock.isValid()) {
            return startStmt.indent();
        } else {
            return QString();
        }
    }

    // Are we inside a parameter list, array or hash?

    TextPosition openingBracketPos = findAnyOpeningBracketBackward(TextPosition(block, 0));

    if (openingBracketPos.isValid()) {
        bool shouldIndent = (openingBracketPos.block == prevStmt.endBlock) ||
                            QRegularExpression(",\\s*$").match(prevStmtContent).hasMatch();

        if ((!isLastCodeColumn(openingBracketPos.block, openingBracketPos.column)) ||
            findAnyOpeningBracketBackward(openingBracketPos).isValid()) {
            // TODO This is alignment, should force using spaces instead of
            // tabs:
            if (shouldIndent) {
                openingBracketPos.column += 1;
                int nextCol = nextNonSpaceColumn(openingBracketPos.block, openingBracketPos.column);
                if (nextCol > 0 && (!isComment(openingBracketPos.block, nextCol))) {
                    openingBracketPos.column = nextCol;
                }
            }

            // Align to the anchor column
            return makeIndentAsColumn(openingBracketPos.block, openingBracketPos.column, width_,
                                      useTabs_);
        } else {
            if (shouldIndent) {
                return increaseIndent(blockIndent(openingBracketPos.block), indentText());
            } else if (isBlockStart(prevStmt)) {
                return increaseIndent(prevStmtIndent, indentText());
            } else {
                return blockIndent(prevStmt.endBlock);
            }
        }
    }

    // Handle indenting of multiline statements.
    if ((prevStmt.endBlock == block.previous() && isBlockContinuing(prevStmt.endBlock)) ||
        isStmtContinuing(prevStmt.endBlock)) {
        if (prevStmt.startBlock == prevStmt.endBlock) {
            if (blockIndent(block).length() > blockIndent(prevStmt.endBlock).length()) {
                // Don't force a specific indent level when aligning manually
                return QString();
            }
            return increaseIndent(increaseIndent(prevStmtIndent, indentText()), indentText());
        } else {
            return blockIndent(prevStmt.endBlock);
        }
    }

    if (isBlockStart(prevStmt) && (!rxBlockEnd.match(prevStmt.content()).hasMatch())) {
        return increaseIndent(prevStmtIndent, indentText());
    }

    if (QRegularExpression("[\\\\[\\\\{]\\s*$").match(prevStmtContent).hasMatch()) {
        return increaseIndent(prevStmtIndent, indentText());
    }

    // Keep current
    return prevStmtIndent;
}

bool IndentAlgRuby::isValidTrigger(QTextBlock block) const {
    if (block.text().isEmpty()) { // new line
        return true;
    }

    QRegularExpressionMatch match = rxUnindent.match(block.text());
    return match.hasMatch() && match.captured(3).isEmpty();
}

} // namespace Qutepart

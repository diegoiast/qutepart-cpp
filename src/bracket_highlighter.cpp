#include <QDebug>

#include "text_block_utils.h"

#include "bracket_highlighter.h"
#include "hl/text_type.h"

namespace Qutepart {

namespace {

const QString START_BRACKETS = "({[";
const QString END_BRACKETS = ")}]";
const QString ALL_BRACKETS = START_BRACKETS + END_BRACKETS;

// Make matched or unmatched QTextEdit.ExtraSelection
QTextEdit::ExtraSelection makeMatchSelection(const TextPosition &pos, bool matched) {
    QTextEdit::ExtraSelection selection;

    if (matched) {
        selection.format.setBackground(Qt::green);
    } else {
        selection.format.setBackground(Qt::red);
    }

    selection.cursor = QTextCursor(pos.block);
    selection.cursor.setPosition(pos.block.position() + pos.column);
    selection.cursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor);

    return selection;
}

} // anonymous namespace

QList<QTextEdit::ExtraSelection> BracketHighlighter::highlightBracket(QChar bracket,
                                                                      const TextPosition &pos) {
    TextPosition matchingPos;

    if (START_BRACKETS.contains(bracket)) {
        matchingPos = findClosingBracketForward(bracket, pos);
    } else {
        matchingPos = findOpeningBracketBackward(bracket, pos);
    }

#if 0 // TODO timeout
    if ( ! matchingPos.isValid()) {
        return QList<QTextEdit::ExtraSelection>();
    }
#endif

    cachedBracket_ = pos;
    cachedMatchingBracket_ = matchingPos;

    QList<QTextEdit::ExtraSelection> result;
    if (matchingPos.isValid()) {
        result.append(makeMatchSelection(pos, true));
        result.append(makeMatchSelection(matchingPos, true));
    } else {
        result.append(makeMatchSelection(pos, false));
    }

    return result;
}

QList<QTextEdit::ExtraSelection> BracketHighlighter::extraSelections(const TextPosition &pos) {
    QString blockText = pos.block.text();

    if (pos.column < blockText.length() && ALL_BRACKETS.contains(blockText[pos.column]) &&
        isCode(pos.block, pos.column)) {
        return highlightBracket(blockText[pos.column], pos);
    } else if (pos.column > 0 && ALL_BRACKETS.contains(blockText[pos.column - 1]) &&
               isCode(pos.block, pos.column - 1)) {
        TextPosition newPos = pos;
        newPos.column -= 1;
        return highlightBracket(blockText[pos.column - 1], newPos);
    } else {
        return QList<QTextEdit::ExtraSelection>();
    }
}

} // namespace Qutepart

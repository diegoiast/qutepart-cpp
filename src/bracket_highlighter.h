#pragma once

#include <QList>
#include <QPlainTextEdit>

#include "text_pos.h"

namespace Qutepart {

class Qutepart;

class BracketHighlighter {
  public:
    BracketHighlighter(Qutepart *q);
    ~BracketHighlighter() = default;

    QList<QTextEdit::ExtraSelection> extraSelections(const TextPosition &pos);
    QTextEdit::ExtraSelection makeMatchSelection(const TextPosition &pos, bool matched);

  private:
    QList<QTextEdit::ExtraSelection> highlightBracket(QChar bracket, const TextPosition &pos);
    TextPosition cachedBracket_;
    TextPosition cachedMatchingBracket_;

    Qutepart *qpart;
};

} // namespace Qutepart

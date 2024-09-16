#pragma once

#include <QList>
#include <QPlainTextEdit>

#include "text_block_utils.h"

namespace Qutepart {

class BracketHighlighter {
  public:
    QList<QTextEdit::ExtraSelection> extraSelections(const TextPosition &pos);

  private:
    QList<QTextEdit::ExtraSelection> highlightBracket(QChar bracket, const TextPosition &pos);
    TextPosition cachedBracket_;
    TextPosition cachedMatchingBracket_;
};

} // namespace Qutepart

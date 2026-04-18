#pragma once

#include <QTextBlock>

namespace Qutepart {

class SyntaxHighlighter;

class SpellChecker {
  public:
    virtual ~SpellChecker() = default;
    virtual void spellCheck(const QTextBlock &block, SyntaxHighlighter *highlighter) = 0;
};

} // namespace Qutepart

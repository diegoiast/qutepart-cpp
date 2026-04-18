#pragma once

#include "../../include/qutepart/spellchecker.h"

namespace Sonnet {
class WordTokenizer;
class Speller;
} // namespace Sonnet

namespace Qutepart {

class SyntaxHighlighter;

class SonnetSpellChecker : public SpellChecker {
  public:
    SonnetSpellChecker();
    ~SonnetSpellChecker() override;
    void spellCheck(const QTextBlock &block, SyntaxHighlighter *highlighter) override;

  private:
    Sonnet::Speller *speller_;
    Sonnet::WordTokenizer *wordTokenizer_;
};

} // namespace Qutepart

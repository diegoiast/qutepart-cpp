#pragma once

#include "../../include/qutepart/spellchecker.h"
#include <QMap>
#include <QTextBlockUserData>

namespace Sonnet {
class WordTokenizer;
class Speller;
class LanguageFilter;
} // namespace Sonnet

namespace Qutepart {

class SyntaxHighlighter;

class SonnetLanguageCache : public QTextBlockUserData {
  public:
    QMap<QPair<int, int>, QString> languages;
    void invalidate(int pos);
    QString languageAtPos(int pos) const;
};

class SonnetSpellChecker : public SpellChecker {
  public:
    SonnetSpellChecker();
    ~SonnetSpellChecker() override;
    void highlightBlock(const QString &text);
    void spellCheck(const QTextBlock &block, SyntaxHighlighter *highlighter) override;

  private:
    Sonnet::Speller *speller_;
    Sonnet::WordTokenizer *wordTokenizer_;
    Sonnet::LanguageFilter *languageFilter_;
};

} // namespace Qutepart

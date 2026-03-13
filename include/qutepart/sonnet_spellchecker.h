#pragma once

#include "../../include/qutepart/spellchecker.h"
#include <QMap>
#include <QSyntaxHighlighter>
#include <QTextBlockUserData>

namespace Sonnet {
class WordTokenizer;
class Speller;
class LanguageFilter;
} // namespace Sonnet

namespace Qutepart {

class SonnetLanguageCache : public QTextBlockUserData {
  public:
    QMap<QPair<int, int>, QString> languages;
    void invalidate(int pos);
    QString languageAtPos(int pos) const;
};

class SonnetSpellChecker : public QSyntaxHighlighter, public SpellChecker {
  public:
    SonnetSpellChecker();
    ~SonnetSpellChecker() override;
    void highlightBlock(const QString &text) override;
    void spellCheck(const QTextBlock &block) override;

  private:
    Sonnet::Speller *speller_;
    Sonnet::WordTokenizer *wordTokenizer_;
    Sonnet::LanguageFilter *languageFilter_;
};

} // namespace Qutepart

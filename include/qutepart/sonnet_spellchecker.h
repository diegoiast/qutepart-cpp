#pragma once

#include <QSet>
#include <QString>

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
    bool isMisspelled(const QString &word) override;
    QStringList suggestions(const QString &word) override;
    void ignoreWord(const QString &word) override;
    void addToPersonalDictionary(const QString &word) override;

  private:
    QSet<QString> ignoredWords_;
    Sonnet::Speller *speller_;
    Sonnet::WordTokenizer *wordTokenizer_;
};

} // namespace Qutepart

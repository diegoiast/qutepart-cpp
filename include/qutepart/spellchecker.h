#pragma once

#include <QStringList>
#include <QTextBlock>

namespace Qutepart {

class SyntaxHighlighter;

class SpellChecker {
  public:
    virtual ~SpellChecker() = default;
    virtual void spellCheck(const QTextBlock &block, SyntaxHighlighter *highlighter) = 0;
    virtual bool isMisspelled(const QString &word) = 0;
    virtual QStringList suggestions(const QString &word) = 0;
    virtual void ignoreWord(const QString &word) = 0;
    virtual void addToPersonalDictionary(const QString &word) = 0;
};

} // namespace Qutepart

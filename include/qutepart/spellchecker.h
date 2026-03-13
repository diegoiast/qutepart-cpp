#pragma once

#include <QTextBlock>

namespace Qutepart {

class SpellChecker {
  public:
    virtual ~SpellChecker() = default;
    virtual void spellCheck(const QTextBlock &block) = 0;
};

} // namespace Qutepart

#pragma once

#include <QString>
#include <QTextBlock>

#include "text_block_utils.h"

#include "indenter.h"

namespace Qutepart {

class IndentAlgPython : public IndentAlgImpl {
  public:
    QString computeSmartIndent(QTextBlock block, int cursorPos) const override;

  private:
    QString computeSmartIndent(const TextPosition &pos) const;
};

} // namespace Qutepart

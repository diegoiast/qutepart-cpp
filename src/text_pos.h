#pragma once

#include <QTextBlock>

namespace Qutepart {

struct TextPosition {
  public:
    inline TextPosition() : column(-1){};

    inline TextPosition(QTextBlock block_, int column_) : block(block_), column(column_){};

    inline bool isValid() const { return block.isValid(); }

    QTextBlock block;
    int column;
};

} // namespace Qutepart

#pragma once

#include <QTextBlock>

namespace Qutepart {

const int BOOMARK_BIT = 0x1;

bool isBookmarked(const QTextBlock &block);
void setBookmarked(QTextBlock &block, bool value);

} // namespace Qutepart

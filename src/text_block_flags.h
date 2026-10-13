/*
 * Copyright (C) 2018-2023 Andrei Kopats
 * Copyright (C) 2023-...  Diego Iastrubni <diegoiast@gmail.com>
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <QTextBlock>
#include <qutepart.h>

#include "text_block_user_data.h"

namespace Qutepart {

/// The block's state bits, or 0 when it carries no user data.
///
/// Defined here rather than in the .cpp so a caller testing several flags pays
/// for one read of the block data instead of one per test. The margin does this
/// for every visible line on every repaint.
void setFlag(QTextBlock &block, int flag, bool value);

inline auto blockState(const QTextBlock &block) -> int {
    auto data = static_cast<TextBlockUserData *>(block.userData());
    if (!data) {
        return 0;
    }
    // -1 is the "flags not computed yet" sentinel, not a set of bits.
    return data->state == -1 ? 0 : data->state;
}

inline bool hasFlag(const QTextBlock &block, int flag) {
    return blockState(block) & flag;
}

/// The block's user data, created if absent. Null only for an invalid block.
auto ensureBlockData(QTextBlock &block) -> TextBlockUserData *;

inline bool isBookmarked(const QTextBlock &block) { return hasFlag(block, BOOMARK_BIT); }
inline void setBookmarked(QTextBlock &block, bool value) { setFlag(block, BOOMARK_BIT, value); }

} // namespace Qutepart

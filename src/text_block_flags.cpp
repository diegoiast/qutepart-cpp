/*
 * Copyright (C) 2018-2023 Andrei Kopats
 * Copyright (C) 2023-...  Diego Iastrubni <diegoiast@gmail.com>
 * SPDX-License-Identifier: MIT
 */

#include "text_block_flags.h"

namespace Qutepart {

bool isBookmarked(const QTextBlock &block) {
    int state = block.userState();
    return state != -1 && state & BOOMARK_BIT;
}

void setBookmarked(QTextBlock &block, bool value) {
    int state = block.userState();
    if (state == -1) {
        state = 0;
    }

    if (value) {
        state |= BOOMARK_BIT;
    } else {
        state &= (~BOOMARK_BIT);
    }

    block.setUserState(state);
}

} // namespace Qutepart

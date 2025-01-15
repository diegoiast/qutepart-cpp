/*
 * Copyright (C) 2018-2023 Andrei Kopats
 * Copyright (C) 2023-...  Diego Iastrubni <diegoiast@gmail.com>
 * SPDX-License-Identifier: MIT
 */

#include "text_block_flags.h"
#include "qutepart.h"

namespace Qutepart {

bool hasFlag(const QTextBlock &block, int flag) {
    int state = block.userState();
    return state != -1 && state & flag;
}

void setFlag(QTextBlock &block, int flag, bool value) {
    int state = block.userState();
    if (state == -1) {
        state = 0;
    }

    if (value) {
        state |= flag;
    } else {
        state &= (~flag);
    }

    block.setUserState(state);
}

} // namespace Qutepart

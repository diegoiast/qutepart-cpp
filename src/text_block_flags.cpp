/*
 * Copyright (C) 2018-2023 Andrei Kopats
 * Copyright (C) 2023-...  Diego Iastrubni <diegoiast@gmail.com>
 * SPDX-License-Identifier: MIT
 */

#include "text_block_flags.h"
#include "text_block_user_data.h"
#include <qutepart.h>

namespace Qutepart {

/// The block's user data, created if absent.
///
/// Kept in step with the copy in qutepart.cpp; both writers of block state need
/// it, and returning early when there is none makes setFlag() drop the write
/// silently - so a marker set on a block that had never carried state used to
/// disappear with nothing to show for it.
auto ensureBlockData(QTextBlock &block) -> TextBlockUserData * {
    if (auto *existing = block.userData()) {
        // dynamic_cast, not static_cast: QTextBlockUserData is Qt's extension
        // point, so another library may own this block. The cost is a couple of
        // loads, on a path that runs per visible block per repaint, and in
        // exchange a foreign owner is reported instead of being reinterpreted
        // and silently corrupted.
        if (auto *ours = dynamic_cast<TextBlockUserData *>(existing)) {
            return ours;
        }
        // Replacing it would discard their data, and adopting it would read
        // fields that are not there. Say so rather than pretend.
        qWarning() << "Qutepart: block carries foreign user data, not adopting it";
        return nullptr;
    }
    if (!block.isValid()) {
        return nullptr;
    }
    auto data = new TextBlockUserData({}, {nullptr});
    block.setUserData(data);
    return data;
}

void setFlag(QTextBlock &block, int flag, bool value) {
    auto data = ensureBlockData(block);
    if (!data) {
        return;
    }
    auto &state = data->state;
    if (state == -1) {
        state = 0;
    }

    if (value) {
        state |= flag;
    } else {
        state &= (~flag);
    }
}

} // namespace Qutepart

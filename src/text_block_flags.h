/*
 * Copyright (C) 2018-2023 Andrei Kopats
 * Copyright (C) 2023-...  Diego Iastrubni <diegoiast@gmail.com>
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <QTextBlock>

namespace Qutepart {

const int BOOMARK_BIT = 0x1;

bool isBookmarked(const QTextBlock &block);
void setBookmarked(QTextBlock &block, bool value);

} // namespace Qutepart

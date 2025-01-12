/*
 * Copyright (C) 2018-2023 Andrei Kopats
 * Copyright (C) 2023-...  Diego Iastrubni <diegoiast@gmail.com>
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <QTextBlock>

namespace Qutepart {

// clang-format off
const int BOOMARK_BIT       = 1 << 0;
const int MODIFIED_BIT      = 1 << 1;
const int WARNING_BIT       = 1 << 2;
const int ERRROR_BIT        = 1 << 3;
const int INFO_BIT          = 1 << 4;
const int BREAKPOINT_BIT    = 1 << 5;
const int EXECUTING_BIT     = 1 << 6;

// Future expansion
const int UNUSED9_BIT  = 1 << 7;
const int UNUSED8_BIT  = 1 << 8;
const int UNUSED7_BIT  = 1 << 9;
const int UNUSED6_BIT  = 1 << 10;
const int UNUSED5_BIT  = 1 << 11;
const int UNUSED4_BIT  = 1 << 12;
const int UNUSED3_BIT  = 1 << 13;
const int UNUSED2_BIT  = 1 << 14;
const int UNUSED1_BIT  = 1 << 15;
// clang-format on

bool hasFlag(const QTextBlock &block, int flag);
void setFlag(QTextBlock &block, int flag, bool value);

inline bool isBookmarked(const QTextBlock &block) { return hasFlag(block, BOOMARK_BIT); }
inline void setBookmarked(QTextBlock &block, bool value) { setFlag(block, BOOMARK_BIT, value); }

} // namespace Qutepart

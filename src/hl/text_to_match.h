/*
 * Copyright (C) 2018-2023 Andrei Kopats
 * Copyright (C) 2023-...  Diego Iastrubni <diegoiast@gmail.com>
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <bitset>

#include <QString>

namespace Qutepart {

/* A set of "word deliminator" characters, with O(1) membership testing.
 * Built once (when a rule's deliminator string is set) and reused on every
 * character checked while extracting a word - avoids rescanning the
 * deliminator string for every character of every candidate word.
 */
class DeliminatorSet {
  public:
    DeliminatorSet() = default;
    explicit DeliminatorSet(const QString &deliminators);

    bool contains(QChar ch) const;

  private:
    std::bitset<128> asciiChars_;
    QString nonAsciiChars_; // deliminators outside ASCII range; expected to be empty in practice
};

/* Peace of text, which shall be matched.
 * Contains pre-calculated and pre-checked data for performance optimization
 */
class TextToMatch {
  public:
    TextToMatch(const QString &text, const QStringList &contextData);

    void shiftOnce();
    void shift(int count);

    bool isEmpty() const;

    QStringView word(const DeliminatorSet &deliminators) const;

    int currentColumnIndex;
    QString wholeLineText;
    QStringView text;
    int textLength;
    bool firstNonSpace;
    bool isWordStart;
    const QStringList *contextData;
};

} // namespace Qutepart

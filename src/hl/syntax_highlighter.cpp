/*
 * Copyright (C) 2018-2023 Andrei Kopats
 * Copyright (C) 2023-...  Diego Iastrubni <diegoiast@gmail.com>
 * SPDX-License-Identifier: MIT
 */

#include <QTextLayout>
#include <Qt>

#include "language.h"
#include "syntax_highlighter.h"
#include "theme.h"
#include "text_type.h"
#include "../../include/qutepart/spellchecker.h"

namespace Qutepart {

SyntaxHighlighter::SyntaxHighlighter(QTextDocument *parent, QSharedPointer<Language> language)
    : QSyntaxHighlighter(parent), language(language) {}

void SyntaxHighlighter::addBlockFormat(int start, int length, const QTextCharFormat &format) {
    if (!isSpellCheckable(start, length)) {
        return;
    }

    for (int i = start; i < start + length; ++i) {
        QTextCharFormat merged = this->format(i);
        merged.merge(format);
        setFormat(i, 1, merged);
    }
}

bool SyntaxHighlighter::isSpellCheckable(int start, int length) const {
    // Language highlighting populates the block's text-type map before the
    // spell checker runs. Only itemData sections marked spellChecking="true"
    // are represented by an upper-case text type in that map.
    if (start < 0 || length <= 0) {
        return false;
    }

    const QTextBlock block = currentBlock();
    for (int column = start; column < start + length; ++column) {
        if (!Qutepart::isSpellCheckable(block, column)) {
            return false;
        }
    }
    return true;
}

SyntaxHighlighter::SyntaxHighlighter(QObject *parent, QSharedPointer<Language> language)
    : QSyntaxHighlighter(parent), language(language) {}

void SyntaxHighlighter::highlightBlock(const QString &) {
    formats.clear();

    auto b = currentBlock();
    auto data = b.userData();
    if (!data) {
        data = new TextBlockUserData({}, {nullptr});
        b.setUserData(data);
    }

    auto state = language->highlightBlock(b, formats);
    for (auto &range : std::as_const(formats)) {
        for (auto i = range.start; i < range.start + range.length; ++i) {
            auto merged = format(i);
            merged.merge(range.format);
            setFormat(i, 1, merged);
        }
    }
    setCurrentBlockState(state);
    if (spellChecker_) {
        spellChecker_->spellCheck(b, this);
    }
}

} // namespace Qutepart

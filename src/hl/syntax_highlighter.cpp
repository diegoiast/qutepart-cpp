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
#include "../../include/qutepart/spellchecker.h"

namespace Qutepart {

SyntaxHighlighter::SyntaxHighlighter(QTextDocument *parent, QSharedPointer<Language> language)
    : QSyntaxHighlighter(parent), language(language) {}

SyntaxHighlighter::SyntaxHighlighter(QObject *parent, QSharedPointer<Language> language)
    : QSyntaxHighlighter(parent), language(language) {}

void SyntaxHighlighter::highlightBlock(const QString &) {
    QVector<QTextLayout::FormatRange> formats;

    auto b = currentBlock();
    auto data = b.userData();
    if (!data) {
        data = new TextBlockUserData({}, {nullptr});
        b.setUserData(data);
    }

    if (spellChecker_) {
        spellChecker_->spellCheck(b, this);
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
}

} // namespace Qutepart

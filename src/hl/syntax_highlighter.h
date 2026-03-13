/*
 * Copyright (C) 2018-2023 Andrei Kopats
 * Copyright (C) 2023-...  Diego Iastrubni <diegoiast@gmail.com>
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <QSyntaxHighlighter>
#include <QTextDocument>

#include "language.h"
#include "text_block_user_data.h"

namespace Qutepart {

class Theme;
class SpellChecker;

class SyntaxHighlighter : public QSyntaxHighlighter {
    Q_OBJECT

  public:
    SyntaxHighlighter(QObject *parent, QSharedPointer<Language> language);
    SyntaxHighlighter(QTextDocument *parent, QSharedPointer<Language> language);

    inline QSharedPointer<Language> getLanguage() const { return language; }
    inline void setTheme(const Theme *t) {
        language->setTheme(t);
        rehighlight();
    }
    void setSpellChecker(SpellChecker *checker) { spellChecker_ = checker; }

  protected:
    void highlightBlock(const QString &text) override;
    QSharedPointer<Language> language;
    SpellChecker *spellChecker_ = nullptr;
};

} // namespace Qutepart

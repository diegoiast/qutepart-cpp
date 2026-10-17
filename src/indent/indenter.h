/*
 * Copyright (C) 2018-2023 Andrei Kopats
 * Copyright (C) 2023-...  Diego Iastrubni <diegoiast@gmail.com>
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <QKeyEvent>
#include <QObject>
#include <QString>
#include <QTextBlock>

#include "alg_impl.h"
#include "qutepart.h"

namespace Qutepart {

class Indenter : public QObject {
  public:
    Indenter(QObject *parent);
    ~Indenter();

    void setAlgorithm(IndentAlg alg);

    QString indentText() const;

    int width() const;
    void setWidth(int);

    bool useTabs() const;
    void setUseTabs(bool);

    void setLanguage(const QString &language);

    /* The marker which starts a single line comment in the current language,
     * like '//' in C++ or '#' in Ruby. Used to continue such a comment when a
     * new line is started.
     */
    void setLineCommentMarker(const QString &marker);

    bool shouldAutoIndentOnEvent(QKeyEvent *event) const;
    bool shouldUnindentWithBackspace(const QTextCursor &cursor) const;
#if 0
    void autoIndentBlock(QTextBlock block, QChar typedKey) const;
#endif
    void indentBlock(QTextBlock block, int cursorPos, int typedKey) const;
    /* Indent a block created by soft line wrapping - it continues the previous
     * line instead of starting a new one.
     */
    void indentWrappedBlock(QTextBlock block, int cursorPos) const;
  public slots:
    void onShortcutIndentAfterCursor(QTextCursor cursor) const;
    void onShortcutUnindentWithBackspace(QTextCursor &cursor) const;

  private:
    IndentAlgImpl *alg_;
    bool useTabs_;
    int width_;
    QString language_;
    QString lineCommentMarker_;
};

} // namespace Qutepart

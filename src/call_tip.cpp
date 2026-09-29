/*
 * Copyright (C) 2023-...  Diego Iastrubni <diegoiast@gmail.com>
 * SPDX-License-Identifier: MIT
 */

#include <algorithm>
#include <QApplication>
#include <QFrame>
#include <QLabel>
#include <QTextDocument>

#include "call_tip.h"
#include "qutepart.h"

namespace Qutepart {

namespace {
constexpr int TIP_VERTICAL_MARGIN = 2;
}

CallTip::CallTip(Qutepart *qpart) : QObject(qpart), qpart_(qpart) {
    /* A call tip describes one invocation of a function; once the caret moves
     * or the document changes it no longer applies, so drop it. Clients that
     * want an updated tip re-show it. */
    connect(qpart, &Qutepart::cursorPositionChanged, this, &CallTip::hideCallTip);
    connect(qpart->document(), &QTextDocument::contentsChange, this,
            [this](int, int, int) { hideCallTip(); });
}

CallTip::~CallTip() = default;

void CallTip::showCallTip(const QString &text) {
    if (!label_) {
        QPalette palette;
        palette.setColor(QPalette::Window, QApplication::palette().color(QPalette::ToolTipBase));
        palette.setColor(QPalette::WindowText, QApplication::palette().color(QPalette::ToolTipText));
        
        label_ = new QLabel(qpart_->viewport());
        label_->setPalette(palette);
        label_->setAutoFillBackground(true);
        label_->setFrameStyle(QFrame::Box);
        label_->setFrameShadow(QFrame::Plain);
        label_->setFont(qpart_->font());
        label_->setTextFormat(Qt::RichText);
        label_->setFocusPolicy(Qt::NoFocus);
        label_->setAttribute(Qt::WA_TransparentForMouseEvents);
    }
    label_->setText(text);
    reposition();
    label_->show();
    label_->raise();
}

void CallTip::updateCallTip(const QString &text) {
    if (!label_ || !label_->isVisible()) {
        showCallTip(text);
        return;
    }
    label_->setText(text);
    reposition();
}

void CallTip::hideCallTip() {
    if (label_) {
        label_->hide();
    }
}

bool CallTip::isVisible() const { return label_ && label_->isVisible(); }

void CallTip::reposition() {
    auto cursorRect = qpart_->QPlainTextEdit::cursorRect(qpart_->textCursor());
    auto parentSize = qpart_->viewport()->size();
    auto hint = label_->sizeHint();
    auto x = cursorRect.left();
    auto y = cursorRect.bottom() + TIP_VERTICAL_MARGIN;
    
    // flip above the cursor line when it does not fit below
    if (y + hint.height() > parentSize.height()) {
        y = cursorRect.top() - hint.height() - TIP_VERTICAL_MARGIN;
    }
    y = std::max(y, 0);
    x = std::max(0, std::min(x, parentSize.width() - hint.width()));
    label_->setGeometry(x, y, hint.width(), hint.height());
}

} // namespace Qutepart
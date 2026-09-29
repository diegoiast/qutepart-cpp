/*
 * Copyright (C) 2023-...  Diego Iastrubni <diegoiast@gmail.com>
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <QObject>

class QLabel;

namespace Qutepart {

class Qutepart;

/* A tooltip-like widget anchored at the current cursor, used to show call tips.
 * Non-interactive: mouse events pass through to the editor.
 */
class CallTip : public QObject {
    Q_OBJECT

  public:
    explicit CallTip(Qutepart *qpart);
    ~CallTip() override;

    /* Show (or refresh) the call tip with `text`. `text` may be HTML. */
    void showCallTip(const QString &text);

    /* Change the shown text, keeping the position. Shows the tip if hidden. */
    void updateCallTip(const QString &text);
    void hideCallTip();
    bool isVisible() const;

  private:
    void reposition();

    Qutepart *qpart_;
    /* Child of the viewport; owned by it. Kept alive between show/hide calls and
     * only ever hidden, so the pointer stays valid. */
    QLabel *label_ = nullptr;
};

} // namespace Qutepart

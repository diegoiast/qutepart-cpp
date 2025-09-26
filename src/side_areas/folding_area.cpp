#include "folding_area.h"
#include "qutepart.h"
#include "text_block_user_data.h"

#include <QPainter>
#include <QTextBlock>
#include <QDebug>
#include <QMouseEvent>

namespace Qutepart {

FoldingArea::FoldingArea(Qutepart *editor) : QWidget(editor), editor_(editor) {
    setMouseTracking(true);
}

QSize FoldingArea::sizeHint() const { return QSize(widthHint(), 0); }

int FoldingArea::widthHint() const {
    return editor_->fontMetrics().horizontalAdvance(QLatin1Char('9')) + 6;
}

void FoldingArea::paintEvent(QPaintEvent *event) {
    QPainter painter(this);
    painter.fillRect(event->rect(), editor_->palette().color(QPalette::Window));

    QTextBlock block = editor_->firstVisibleBlock();
    int top = qRound(editor_->blockBoundingRect(block).translated(editor_->contentOffset()).top());
    int bottom = top + qRound(editor_->blockBoundingRect(block).height());

    while (block.isValid() && top <= event->rect().bottom()) {
        if (block.isVisible() && bottom >= event->rect().top()) {
            TextBlockUserData *data = static_cast<TextBlockUserData *>(block.userData());
            if (data) {
                // qDebug() << "Block" << block.blockNumber() << "folding level" <<
                // data->folding.level;
            }

            QTextBlock prevBlock = block.previous();
            TextBlockUserData *prevData = nullptr;
            if (prevBlock.isValid()) {
                prevData = static_cast<TextBlockUserData *>(prevBlock.userData());
            }

            int prevLevel = 0;
            if (prevData) {
                prevLevel = prevData->folding.level;
            }

            if (data && data->folding.level > prevLevel) {
                // qDebug() << "  - Drawing folding marker for block" << block.blockNumber();
                painter.setPen(editor_->palette().color(QPalette::Text));
                QRect r(1, top, width() - 2, editor_->fontMetrics().height());
                painter.drawRect(r);

                if (block.next().isVisible()) {
                    painter.drawText(r, Qt::AlignCenter, "-");
                } else {
                    painter.drawText(r, Qt::AlignCenter, "+");
                }
            }
        }

        block = block.next();
        top = bottom;
        bottom = top + qRound(editor_->blockBoundingRect(block).height());
    }
}

QTextBlock FoldingArea::blockAt(const QPoint& pos) const {
    QTextBlock block = editor_->firstVisibleBlock();
    if ( ! block.isValid()) {
        return QTextBlock();
    }

    int top = qRound(editor_->blockBoundingRect(block).translated(editor_->contentOffset()).top());
    int bottom = top + qRound(editor_->blockBoundingRect(block).height());

    while (block.isValid() && top <= pos.y()) {
        if (block.isVisible() && bottom >= pos.y()) {
            return block;
        }
        block = block.next();
        top = bottom;
        bottom = top + qRound(editor_->blockBoundingRect(block).height());
    }

    return QTextBlock();
}

void FoldingArea::mouseMoveEvent(QMouseEvent *event)
{
    QTextBlock block = blockAt(event->pos());
    if (block.isValid()) {
        TextBlockUserData *data = static_cast<TextBlockUserData *>(block.userData());
        if (data && data->folding.level > 0) {
            setCursor(Qt::PointingHandCursor);
        } else {
            unsetCursor();
        }
    } else {
        unsetCursor();
    }

    QWidget::mouseMoveEvent(event);
}

void FoldingArea::leaveEvent(QEvent *event)
{
    unsetCursor();
    QWidget::leaveEvent(event);
}

void FoldingArea::mousePressEvent(QMouseEvent *event) {
    qDebug() << "FoldingArea::mousePressEvent";
    if (event->button() == Qt::LeftButton) {
        QTextBlock block = blockAt(event->pos());
        if (block.isValid()) {
            TextBlockUserData *data = static_cast<TextBlockUserData *>(block.userData());
            if (data && data->folding.level > 0) {
                qDebug() << "FoldingArea: Emitting foldClicked for block" << block.blockNumber();
                emit foldClicked(block.blockNumber());
                event->accept();
                return;
            }
        }
    }
    QWidget::mousePressEvent(event);
}


void FoldingArea::onUpdateRequest(const QRect &rect, int dy)
{
    if (dy) {
        scroll(0, dy);
    } else {
        update(0, rect.y(), width(), rect.height());
    }
}

} // namespace Qutepart

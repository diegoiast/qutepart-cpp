#include <QPainter>
#include <QTextBlock>
#include <QIcon>
#include <QPaintEvent>
#include <QScrollBar>
#include <QDebug>

#include "qutepart.h"
#include "text_block_flags.h"

#include "side_areas.h"

namespace Qutepart {

namespace {

const int LEFT_LINE_NUM_MARGIN = 5;
const int RIGHT_LINE_NUM_MARGIN = 3;

const int MARK_MARGIN = 1;

}

SideArea::SideArea(Qutepart *textEdit):
    QWidget(textEdit),
    qpart_(textEdit)
{
    connect(textEdit, &Qutepart::updateRequest, this, &SideArea::onTextEditUpdateRequest);
}

void SideArea::onTextEditUpdateRequest(const QRect &rect, int dy) {
    if (dy) {
        scroll(0, dy);
    } else {
        update(0, rect.y(), width(), rect.height());
    }

    if (rect.contains(qpart_->viewport()->rect()))
        updateWidth();
}

LineNumberArea::LineNumberArea(Qutepart* textEdit):
    SideArea(textEdit)
{
    resize(widthHint(), height());
    connect(textEdit->document(), &QTextDocument::blockCountChanged, this, &LineNumberArea::updateWidth);
    updateWidth();
}

int LineNumberArea::widthHint() const {
    int lines = std::max(1, qpart_->document()->blockCount());
    int digits = QString("%1").arg(lines).length();

    return LEFT_LINE_NUM_MARGIN + qpart_->fontMetrics().horizontalAdvance('9') * digits + RIGHT_LINE_NUM_MARGIN;
}

void LineNumberArea::updateWidth() {
    int newWidth = widthHint();
    if (newWidth != width()) {
        resize(newWidth, height());
        emit(widthChanged());
    }

    update();
}

void LineNumberArea::paintEvent(QPaintEvent* event) {
    auto palette = this->palette();
    auto background = palette.color(QPalette::AlternateBase);
    auto foreground = palette.color(QPalette::Text);
    auto wrapColor = palette.color(QPalette::Dark);
    QPainter painter(this);
    painter.fillRect(event->rect(), background);
    painter.setPen(foreground);

    auto currentBlock = qpart_->textCursor().block().fragmentIndex();
    QTextBlock block = qpart_->firstVisibleBlock();
    int blockNumber = block.blockNumber();
    int top = int(qpart_->blockBoundingGeometry(block).translated(qpart_->contentOffset()).top());
    int bottom = top + int(qpart_->blockBoundingRect(block).height());
    int singleBlockHeight = qpart_->cursorRect(block, 0, 0).height();

    QRectF boundingRect = qpart_->blockBoundingRect(block);
    int availableWidth = width() - RIGHT_LINE_NUM_MARGIN - LEFT_LINE_NUM_MARGIN;
    int availableHeight = qpart_->fontMetrics().height();
    while (block.isValid() && top <= event->rect().bottom()) {
        if (block.isVisible() && bottom >= event->rect().top()) {
            QString number = QString("%1").arg(blockNumber + 1);
            QFont font = painter.font();
            font.setBold(block.fragmentIndex() == currentBlock);
            painter.setFont(font);
            painter.drawText(LEFT_LINE_NUM_MARGIN, top,
                             availableWidth, availableHeight,
                             Qt::AlignRight, number);
            if (boundingRect.height() >= singleBlockHeight * 2) {  // wrapped block
                painter.fillRect(1, top + singleBlockHeight,
                                 width() - 2, boundingRect.height() - singleBlockHeight - 2,
                                 wrapColor);
            }
        }

        block = block.next();
        boundingRect = qpart_->blockBoundingRect(block);
        top = bottom;
        bottom = top + int(boundingRect.height());
        blockNumber++;
    }
}

void LineNumberArea::changeEvent(QEvent* event) {
    if (event->type() == QEvent::FontChange) {
        updateWidth();
    }
}


MarkArea::MarkArea(Qutepart* textEdit):
    SideArea(textEdit)
{
#if 0
    setMouseTracking(true);
#endif

    bookmarkPixmap_ = loadIcon("emblem-favorite");
    // self._lintPixmaps = {qpart.LINT_ERROR: self._loadIcon('emblem-error'),
    //                      qpart.LINT_WARNING: self._loadIcon('emblem-warning'),
    //                      qpart.LINT_NOTE: self._loadIcon('emblem-information')}

    connect(textEdit->document(), &QTextDocument::blockCountChanged, [this] {this->update();});
    connect(textEdit->verticalScrollBar(), &QScrollBar::valueChanged, [this] {this->update();});
}

QPixmap MarkArea::loadIcon(const QString& name) const {
    QIcon icon = QIcon::fromTheme(name);
    int size = qpart_->cursorRect(qpart_->document()->begin(), 0, 0).height() - 6;
    return icon.pixmap(size, size);  // This also works with Qt.AA_UseHighDpiPixmaps
}

int MarkArea::widthHint() const {
    return MARK_MARGIN + bookmarkPixmap_.width() + MARK_MARGIN;
}

void MarkArea::paintEvent(QPaintEvent* event) {
    QPainter painter(this);
    painter.fillRect(event->rect(), palette().color(QPalette::AlternateBase));

    QTextBlock block = qpart_->firstVisibleBlock();
    QRectF blockBoundingGeometry = qpart_->blockBoundingGeometry(block).translated(qpart_->contentOffset());
    int top = blockBoundingGeometry.top();

    while (block.isValid() && top <= event->rect().bottom()) {
        int height = qpart_->blockBoundingGeometry(block).height();
        int bottom = top + height;

        if (block.isVisible() && bottom >= event->rect().top()) {
#if 0 // TODO linter marks
            if block.blockNumber() in self.qpart_.lintMarks:
                msgType, msgText = self.qpart_.lintMarks[block.blockNumber()]
                pixMap = self._lintPixmaps[msgType]
                yPos = top + ((height - pixMap.height()) / 2)  # centered
                painter.drawPixmap(0, yPos, pixMap)
#endif

            if (isBookmarked(block)) {
                int yPos = top + ((height - bookmarkPixmap_.height()) / 2);  // centered
                painter.drawPixmap(0, yPos, bookmarkPixmap_);
            }
        }

        top += height;
        block = block.next();
    }
}

#if 0  // TODO linter marks
void MarkArea::mouseMoveEvent(QMouseEvent* event) {
    blockNumber = self.qpart_.cursorForPosition(event.pos()).blockNumber()
    if blockNumber in self.qpart_._lintMarks:
        msgType, msgText = self.qpart_._lintMarks[blockNumber]
        QToolTip.showText(event.globalPos(), msgText)
    else:
        QToolTip.hideText()

    return QWidget::mouseMoveEvent(event);
}
#endif

}  // namespace Qutepart

/*
 * Copyright (C) 2018-2023 Andrei Kopats
 * Copyright (C) 2023-...  Diego Iastrubni <diegoiast@gmail.com>
 * SPDX-License-Identifier: MIT
 */

#include <QDebug>
#include <QIcon>
#include <QPaintEvent>
#include <QPainter>
#include <QScrollBar>
#include <QTextBlock>

#include "qutepart.h"
#include "text_block_flags.h"
#include "theme.h"

#include "side_areas.h"
#include "text_block_user_data.h"

namespace Qutepart {

namespace {

const int LEFT_LINE_NUM_MARGIN = 5;
const int RIGHT_LINE_NUM_MARGIN = 3;

const int MARK_MARGIN = 1;

} // namespace

SideArea::SideArea(Qutepart *textEdit) : QWidget(textEdit), qpart_(textEdit) {
    connect(textEdit, &Qutepart::updateRequest, this, &SideArea::onTextEditUpdateRequest);
}

void SideArea::onTextEditUpdateRequest(const QRect &rect, int dy) {
    if (dy) {
        scroll(0, dy);
    } else {
        update(0, rect.y(), width(), rect.height());
    }

    if (rect.contains(qpart_->viewport()->rect())) {
        updateWidth();
    }
}

LineNumberArea::LineNumberArea(Qutepart *textEdit) : SideArea(textEdit) {
    resize(widthHint(), height());
    connect(textEdit->document(), &QTextDocument::blockCountChanged, this,
            &LineNumberArea::updateWidth);
    updateWidth();
}

int LineNumberArea::widthHint() const {
    int lines = std::max(1, qpart_->document()->blockCount());
    int digits = QString("%1").arg(lines).length();

    return LEFT_LINE_NUM_MARGIN + qpart_->fontMetrics().horizontalAdvance('9') * digits +
           RIGHT_LINE_NUM_MARGIN;
}

void LineNumberArea::updateWidth() {
    int newWidth = widthHint();
    if (newWidth != width()) {
        resize(newWidth, height());
        emit(widthChanged());
    }

    update();
}

void LineNumberArea::paintEvent(QPaintEvent *event) {
    auto palette = this->palette();
    auto background = palette.color(QPalette::AlternateBase);
    auto foreground = palette.color(QPalette::Text);
    auto wrapColor = palette.color(QPalette::Dark);
    auto modifiedColor = palette.color(QPalette::Accent);

    if (qpart_) {
        if (auto theme = qpart_->getTheme()) {
            if (theme->getEditorColors().contains(Theme::Colors::IconBorder)) {
                background = theme->getEditorColors()[Theme::Colors::IconBorder];
                wrapColor = background;
            }
            if (theme->getEditorColors().contains(Theme::Colors::LineNumbers)) {
                foreground = theme->getEditorColors()[Theme::Colors::LineNumbers];
            }
            if (theme->getEditorColors().contains(Theme::Colors::ModifiedLines)) {
                modifiedColor = theme->getEditorColors()[Theme::Colors::ModifiedLines];
            }
        }
    }

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

            if (block.fragmentIndex() == currentBlock) {
                painter.setPen(qpart_->currentLineNumberColor);
                font.setBold(block.fragmentIndex() == currentBlock);
            }
            painter.setFont(font);
            painter.drawText(LEFT_LINE_NUM_MARGIN, top, availableWidth, availableHeight,
                             Qt::AlignRight, number);
            if (boundingRect.height() >= singleBlockHeight * 2) { // wrapped block
                painter.fillRect(1, top + singleBlockHeight, width() - 2,
                                 boundingRect.height() - singleBlockHeight - 2, wrapColor);
            }
            if (block.fragmentIndex() == currentBlock) {
                painter.setPen(foreground);
                font.setBold(false);
                painter.setFont(font);
            }
        }
        
        auto data = static_cast<TextBlockUserData*>(block.userData());
        if (!data || data->metaData.modified) {
            painter.fillRect( width()-3, top, 2, availableHeight, modifiedColor);
        }
        
        block = block.next();
        boundingRect = qpart_->blockBoundingRect(block);
        top = bottom;
        bottom = top + int(boundingRect.height());
        blockNumber++;
    }
}

void LineNumberArea::changeEvent(QEvent *event) {
    if (event->type() == QEvent::FontChange) {
        updateWidth();
    }
}

MarkArea::MarkArea(Qutepart *textEdit) : SideArea(textEdit) {
#if 0
    setMouseTracking(true);
#endif

    bookmarkPixmap_ = loadIcon("emblem-favorite");
    // self._lintPixmaps = {qpart.LINT_ERROR: self._loadIcon('emblem-error'),
    //                      qpart.LINT_WARNING:
    //                      self._loadIcon('emblem-warning'), qpart.LINT_NOTE:
    //                      self._loadIcon('emblem-information')}

    connect(textEdit->document(), &QTextDocument::blockCountChanged, [this] { this->update(); });
    connect(textEdit->verticalScrollBar(), &QScrollBar::valueChanged, [this] { this->update(); });
}

QPixmap MarkArea::loadIcon(const QString &name) const {
    QIcon icon = QIcon::fromTheme(name);
    int size = qpart_->cursorRect(qpart_->document()->begin(), 0, 0).height() - 6;
    return icon.pixmap(size,
                       size); // This also works with Qt.AA_UseHighDpiPixmaps
}

int MarkArea::widthHint() const { return MARK_MARGIN + bookmarkPixmap_.width() + MARK_MARGIN; }

void MarkArea::paintEvent(QPaintEvent *event) {
    QPainter painter(this);
    auto backgruoundColor = palette().color(QPalette::AlternateBase);
    if (auto theme = qpart_->getTheme()) {
        if (theme->getEditorColors().contains(Theme::Colors::IconBorder)) {
            backgruoundColor = theme->getEditorColors()[Theme::Colors::IconBorder];
        }
    }

    painter.fillRect(event->rect(), backgruoundColor);

    QTextBlock block = qpart_->firstVisibleBlock();
    QRectF blockBoundingGeometry =
        qpart_->blockBoundingGeometry(block).translated(qpart_->contentOffset());
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
                int yPos = top + ((height - bookmarkPixmap_.height()) / 2); // centered
                painter.drawPixmap(0, yPos, bookmarkPixmap_);
            }
        }

        top += height;
        block = block.next();
    }
}

#if 0 // TODO linter marks
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

Minimap::Minimap(Qutepart *textEdit) : SideArea(textEdit) {
    // TODO?
}

int Minimap::widthHint() const { return 150; }

void Minimap::mouseMoveEvent(QMouseEvent *event) {
    if (isDragging) {
        updateScroll(event->pos());
    }
}

void Minimap::mousePressEvent(QMouseEvent *event) {
    isDragging = true;
    updateScroll(event->pos());
}

void Minimap::mouseReleaseEvent(QMouseEvent *) { isDragging = false; }

void Minimap::wheelEvent(QWheelEvent *event) {
    auto totalLines = qpart_->document()->blockCount();
    auto delta = event->angleDelta().y();
    auto linesToScroll = delta / 120;
    auto currentLine = qpart_->verticalScrollBar()->value();
    auto newLine = qBound(0, currentLine - linesToScroll, totalLines - 1);

    qpart_->verticalScrollBar()->setValue(newLine);
    event->accept();
}

void Minimap::paintEvent(QPaintEvent *event) {
    if (!qpart_) {
        QWidget::paintEvent(event);
        return;
    }

    QPainter painter(this);
    auto isLargeDocument = qpart_->document()->blockCount() > 10000;
    auto palette = this->palette();
    auto background = palette.color(QPalette::AlternateBase);
    if (auto theme = qpart_->getTheme()) {
        if (theme->getEditorColors().contains(Theme::Colors::IconBorder)) {
            background = theme->getEditorColors()[Theme::Colors::IconBorder];
        }
    }
    painter.fillRect(event->rect(), background);    
    drawMinimapText(&painter, isLargeDocument);
}

QFont Minimap::minimapFont() const {
    QFont font = this->font();
    font.setPointSizeF(2);
    return font;
}

void Minimap::updateScroll(const QPoint &pos) {
    auto doc = qpart_->document();
    auto totalLines = doc->blockCount();
    auto minimapContentHeight = totalLines * lineHeight;
    auto minimapVisibleHeight = height();

    auto minimapOffset = 0;
    if (minimapContentHeight > minimapVisibleHeight) {
        auto viewportStartLine = qpart_->verticalScrollBar()->value();
        auto scrollRatio = static_cast<float>(viewportStartLine) / totalLines;
        minimapOffset =
            static_cast<int>(scrollRatio * (minimapContentHeight - minimapVisibleHeight));
    }

    auto clickedLine = static_cast<int>((pos.y() + minimapOffset) / lineHeight);
    clickedLine = qBound(0, clickedLine, totalLines - 1); // Ensure within bounds

    // Center the clicked line in the viewport
    auto visibleLines = qpart_->viewport()->height() / qpart_->fontMetrics().height();
    auto scrollToLine = qMax(0, clickedLine - visibleLines / 2);
    auto cursor = QTextCursor(doc->findBlockByNumber(clickedLine));
    qpart_->setTextCursor(cursor);
    qpart_->verticalScrollBar()->setValue(scrollToLine);
}

void Minimap::drawMinimapText(QPainter *painter, bool simple) {
    if (!qpart_) {
        return;
    }
    auto minimapArea = rect();
    auto doc = qpart_->document();
    auto block = doc->firstBlock();
    auto totalLines = doc->blockCount();
    auto viewportLines = qpart_->viewport()->height() / qpart_->fontMetrics().height();
    auto viewportStartLine = qpart_->verticalScrollBar()->value();
    auto minimapContentHeight = totalLines * lineHeight;
    auto minimapVisibleHeight = minimapArea.height();

    auto minimapOffset = 0;
    if (minimapContentHeight > minimapVisibleHeight) {
        auto scrollRatio = static_cast<float>(viewportStartLine) / totalLines;
        minimapOffset =
            static_cast<int>(scrollRatio * (minimapContentHeight - minimapVisibleHeight));
        minimapOffset = std::min(minimapOffset, minimapContentHeight - minimapVisibleHeight);
    }

    auto viewportStartY = viewportStartLine * lineHeight - minimapOffset;
    auto viewportHeight = viewportLines * lineHeight;
    auto viewportRect =
        QRect(minimapArea.left(),
              std::max(0, std::min(viewportStartY, minimapVisibleHeight - viewportHeight)),
              minimapArea.width(), std::min(viewportHeight, minimapArea.height()));

    auto currentLineNumber = qpart_->textCursor().blockNumber();
    auto currentLineY = currentLineNumber * lineHeight - minimapOffset;
    auto currentLineRect = QRect(
        minimapArea.left(), std::max(0, std::min(currentLineY, minimapVisibleHeight - lineHeight)),
        minimapArea.width(), lineHeight);

    auto palette = qpart_->palette();
    auto textColor = palette.color(QPalette::Text);

    auto minimapBackground = palette.color(QPalette::AlternateBase);
    if (auto theme = qpart_->getTheme()) {
        if (theme->getEditorColors().contains(Theme::Colors::IconBorder)) {
            minimapBackground = theme->getEditorColors()[Theme::Colors::IconBorder];
        }
    }

    if (minimapBackground.lightnessF() < 0.5) {
        minimapBackground = minimapBackground.lighter(135);
    } else {
        minimapBackground = minimapBackground.darker(125);
    }
    if (minimapBackground == Qt::black) {
        minimapBackground = QColor(30, 30, 30);
    }
    if (minimapBackground == Qt::white) {
        minimapBackground = QColor(225, 225, 225);
    }
    painter->save();
    painter->fillRect(viewportRect, minimapBackground);
    painter->fillRect(currentLineRect, qpart_->currentLineColor());
    painter->setFont(minimapFont());

    auto lineNumber = 0;
    while (block.isValid()) {
        auto y = lineNumber * lineHeight - minimapOffset;
        if (y >= minimapArea.height()) {
            break;
        }
        if (lineNumber == currentLineNumber) {
            painter->setPen(Qt::NoPen);
            painter->setBrush(qpart_->currentLineColor());
            painter->drawRect(minimapArea.left(), y, minimapArea.width(), lineHeight);
        }
        painter->setPen(textColor);
        if (simple) {
            auto lineText = block.text();
            for (auto charIndex = 0; charIndex < lineText.length(); ++charIndex) {
                auto dotX = minimapArea.left() + charIndex * charWidth;
                if (dotX >= minimapArea.right()) {
                    break;
                }
                auto isDrawable =
                    lineText.at(charIndex).isLetterOrNumber() || lineText.at(charIndex).isPunct();
                if (isDrawable) {
                    painter->drawPoint(dotX, y);
                }
            }
        } else {
            auto padding = 5;
            auto textRect = QRectF(minimapArea.left() + padding, y,
                                   minimapArea.width() - padding * 2, lineHeight);
            painter->drawText(textRect, Qt::AlignLeft, block.text());
        }
        block = block.next();
        lineNumber++;
    }

    painter->restore();
}

} // namespace Qutepart

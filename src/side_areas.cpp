/*
 * Copyright (C) 2018-2023 Andrei Kopats
 * Copyright (C) 2023-...  Diego Iastrubni <diegoiast@gmail.com>
 * SPDX-License-Identifier: MIT
 */

#include <QApplication>
#include <QDebug>
#include <QIcon>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QPainter>
#include <QScrollBar>
#include <QSet>
#include <QTextBlock>
#include <QToolTip>
#include <algorithm>

#include "qutepart.h"
#include "text_block_flags.h"
#include "theme.h"

#include "side_areas.h"
#include "text_block_user_data.h"

namespace Qutepart {

namespace {

const int LEFT_LINE_NUM_MARGIN = 5;
const int RIGHT_LINE_NUM_MARGIN = 3;

auto static blendColors(const QColor &color1, const QColor &color2, float r = 0.5) -> QColor {
    if (!color2.isValid()) {
        return color1;
    }
    if (!color1.isValid()) {
        return color2;
    }
    return QColor((1 - r) * color1.red() + color2.red() * r,
                  (1 - r) * color1.green() + color2.green() * r,
                  (1 - r) * color1.blue() + color2.blue() * r, 255);
}

auto static countDigits(int n) -> int {
    if (n == 0) {
        return 1;
    }
    return floor(log10(abs(n))) + 1;
}

} // namespace

SideArea::SideArea(Qutepart *textEdit) : QWidget(textEdit), qpart_(textEdit) {
    connect(textEdit, &Qutepart::updateRequest, this, &SideArea::onTextEditUpdateRequest);
}

void SideArea::wheelEvent(QWheelEvent *event) {
    auto totalLines = qpart_->document()->blockCount();
    auto delta = event->angleDelta().y();
    auto linesToScroll = delta / 120;
    auto currentLine = qpart_->verticalScrollBar()->value();
    auto newLine = qBound(0, currentLine - linesToScroll, totalLines - 1);

    qpart_->verticalScrollBar()->setValue(newLine);
    event->accept();
}

void SideArea::mouseMoveEvent(QMouseEvent *event) {
    QWidget::mouseMoveEvent(event);
    auto cursor = qpart_->cursorForPosition(event->pos());
    auto block = cursor.block();
    auto line = block.blockNumber();

    if (line != this->lastHoeveredLine) {
        lastHoeveredLine = line;
        auto blockData = static_cast<TextBlockUserData *>(block.userData());
        if (blockData && !blockData->metaData.message.isEmpty()) {
            auto lines = QStringList();
            auto lineLength = 100;
            for (auto &s : blockData->metaData.message.split("\n")) {
                for (auto i = 0u; i < s.length(); i += lineLength) {
                    lines.append(s.mid(i, lineLength));
                }
            }
            auto message = lines.join("\n");
            auto fixedMessage = QString("<pre><p style='white-space:pre'>%1</p></pre>")
                                    .arg(message.replace("\n", "<br>"));
            QToolTip::showText(event->globalPosition().toPoint(), fixedMessage, qpart_);

        } else {
            QToolTip::hideText();
        }
    }
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
    setMouseTracking(true);
    connect(textEdit->document(), &QTextDocument::blockCountChanged, this,
            &LineNumberArea::updateWidth);
    updateWidth();
}

int LineNumberArea::widthHint() const {
    auto lines = std::max(1, qpart_->document()->blockCount());
    auto digits = std::max(4, countDigits(lines));

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
    auto block = qpart_->firstVisibleBlock();
    auto blockNumber = block.blockNumber();
    auto top = int(qpart_->blockBoundingRect(block).translated(qpart_->contentOffset()).top());
    auto bottom = top + int(qpart_->blockBoundingRect(block).height());
    auto singleBlockHeight = qpart_->cursorRect(block, 0, 0).height();
    auto boundingRect = qpart_->blockBoundingRect(block);
    auto availableWidth = width() - RIGHT_LINE_NUM_MARGIN - LEFT_LINE_NUM_MARGIN;
    auto availableHeight = qpart_->fontMetrics().height();
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

        if (hasFlag(block, MODIFIED_BIT)) {
            painter.fillRect(width() - 3, top, 2, availableHeight, modifiedColor);
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
    setMouseTracking(true);
    bookmarkPixmap_ = QIcon::fromTheme("emblem-favorite");
    connect(textEdit->document(), &QTextDocument::blockCountChanged, [this] { this->update(); });
    connect(textEdit->verticalScrollBar(), &QScrollBar::valueChanged, [this] { this->update(); });
    scaledIconCache.clear();
}

void MarkArea::changeEvent(QEvent *event) {
    if (event->type() == QEvent::IconTextChange) {
        scaledIconCache.clear();
    }
    QWidget::changeEvent(event);
}

int MarkArea::widthHint() const {
    return qpart_->cursorRect(qpart_->document()->begin(), 0, 0).height();
}

void MarkArea::paintEvent(QPaintEvent *event) {
    QPainter painter(this);
    auto backgroundColor = palette().color(QPalette::AlternateBase);
    if (auto theme = qpart_->getTheme()) {
        auto const &colors = theme->getEditorColors();
        if (colors.contains(Theme::Colors::IconBorder)) {
            backgroundColor = colors[Theme::Colors::IconBorder];
        }
    }
    painter.fillRect(event->rect(), backgroundColor);

    auto block = qpart_->firstVisibleBlock();
    auto blockBoundingRect = qpart_->blockBoundingRect(block).translated(qpart_->contentOffset());
    auto top = blockBoundingRect.top();

    while (block.isValid() && top <= event->rect().bottom()) {
        auto height = qpart_->blockBoundingRect(block).height();
        auto bottom = top + height;

        if (block.isVisible() && bottom >= event->rect().top()) {
            struct MarkInfo {
                bool active;
                int bit;
                QColor color;
            };
            const MarkInfo marks[] = {
                {hasFlag(block, ERROR_BIT), ERROR_BIT, QColor(Qt::red).lighter(170)},
                {hasFlag(block, WARNING_BIT), WARNING_BIT, QColor(Qt::yellow).lighter(150)},
                {hasFlag(block, INFO_BIT), INFO_BIT, QColor(Qt::cyan).lighter(170)}};

            for (auto const &mark : marks) {
                if (!mark.active) {
                    continue;
                }
                auto pixmap = getCachedIcon(iconForStatus(mark.bit), width(), scaledIconCache);
                auto xPos = 0;
                auto yPos = top;
                painter.drawPixmap(xPos, yPos, pixmap);
            }

            if (isBookmarked(block)) {
                auto pixmap = getCachedIcon(bookmarkPixmap_, width(), scaledIconCache);
                auto xPos = 0;
                auto yPos = top;
                painter.drawPixmap(xPos, yPos, pixmap);
            }
        }

        top += height;
        block = block.next();
    }
}

QPixmap MarkArea::getCachedIcon(QIcon icon, int targetSize, QHash<QString, QPixmap> &cache) {
    QString key = QString::number(icon.cacheKey()) + "_" + QString::number(targetSize);
    if (cache.contains(key)) {
        return cache.value(key);
    }

    QSize chosenSize;
    auto const dpr = qpart_->devicePixelRatioF();
    auto const iconSize = qRound(targetSize / dpr);
    auto availableSizes = icon.availableSizes();
    if (!availableSizes.isEmpty()) {
        chosenSize = availableSizes.first();
        for (auto s : std::as_const(availableSizes)) {
            if (s.width() <= iconSize && s.height() <= iconSize) {
                chosenSize = s;
            } else {
                break;
            }
        }
        if (chosenSize.width() > iconSize && availableSizes.first().width() > iconSize) {
            chosenSize = availableSizes.first();
        }
        if (chosenSize.width() < iconSize && availableSizes.last().width() < iconSize) {
            chosenSize = availableSizes.last();
        }
    } else {
        chosenSize = QSize(iconSize, iconSize);
    }

    auto iconPixmap = icon.pixmap(chosenSize);
    iconPixmap.setDevicePixelRatio(dpr);
    auto actualSize = iconPixmap.size() / iconPixmap.devicePixelRatio();
    if (actualSize.width() > targetSize || actualSize.height() > targetSize) {
        auto scaledSize = targetSize * dpr;
        iconPixmap = iconPixmap.scaled(scaledSize, scaledSize, Qt::KeepAspectRatio,
                                       Qt::SmoothTransformation);
        iconPixmap.setDevicePixelRatio(dpr);
        actualSize = iconPixmap.size() / iconPixmap.devicePixelRatio();
    }

    QPixmap finalPixmap(targetSize * dpr, targetSize * dpr);
    QPoint topLeft((targetSize - actualSize.width()) / 2, (targetSize - actualSize.height()) / 2);
    finalPixmap.setDevicePixelRatio(dpr);
    finalPixmap.fill(Qt::transparent);
    {
        QPainter painter(&finalPixmap);
        painter.drawPixmap(topLeft, iconPixmap);
    }
    cache.insert(key, finalPixmap);
    return finalPixmap;
}

QPixmap MarkArea::getCachedPixmap(QPixmap pixmap, int targetSize, QHash<QString, QPixmap> &cache) {
    QString key = QString::number(pixmap.cacheKey()) + "_" + QString::number(targetSize);
    if (cache.contains(key)) {
        return cache.value(key);
    } else {
        QPixmap pixmapToReturn;
        if (targetSize < pixmap.width() || targetSize < pixmap.height()) {
            pixmapToReturn = pixmap.scaled(targetSize, targetSize, Qt::KeepAspectRatio,
                                           Qt::SmoothTransformation);
        } else {
            pixmapToReturn = pixmap;
        }
        qDebug() << "Getting icon of size " << targetSize;
        cache.insert(key, pixmapToReturn);
        return pixmapToReturn;
    }
}

Minimap::Minimap(Qutepart *textEdit) : SideArea(textEdit) {
    connect(textEdit->document(), &QTextDocument::blockCountChanged, this,
            [this]() { invalidateCache(); });
    cacheDirty_ = true;
}

int Minimap::widthHint() const { return 150; }

void Minimap::mouseMoveEvent(QMouseEvent *event) {
    if (isDragging) {
        updateScroll(event->pos());
    }
}

void Minimap::mousePressEvent(QMouseEvent *event) {
    if (event->button() != Qt::LeftButton) {
        return;
    }
    isDragging = true;
    auto rect = viewportRect();
    if (rect.isValid() && rect.contains(event->pos())) {
        dragOffset = event->pos().y() - rect.top();
        dragCenter = false;
    } else {
        // Click outside thumb: first line at click position
        dragOffset = rect.height() / 2;
        dragCenter = true;
    }
    updateScroll(event->pos());
}

void Minimap::mouseReleaseEvent(QMouseEvent *) {
    isDragging = false;
    dragCenter = false;
}

void Minimap::invalidateCache() { cacheDirty_ = true; }

void Minimap::ensureCache() const {
    auto doc = qpart_ ? qpart_->document() : nullptr;
    if (!doc) {
        return;
    }
    if (!isVisible() || doc->blockCount() > 20000) {
        if (!visibleCache_.isEmpty() || cacheDirty_) {
            visibleCache_.clear();
            cachedBlockCount_ = doc->blockCount();
            cacheDirty_ = false;
        }
        return;
    }
    auto curCount = doc->blockCount();
    if (!cacheDirty_ && cachedBlockCount_ == curCount && !visibleCache_.isEmpty()) {
        return;
    }
    if (!cacheDirty_ && cachedBlockCount_ == curCount) {
        return;
    }
    if (!cacheDirty_ && curCount == cachedBlockCount_ + 1 && !visibleCache_.isEmpty()) {
        auto lastBlock = doc->findBlockByNumber(curCount - 1);
        if (lastBlock.isValid()) {
            auto lastCachedNum = visibleCache_.isEmpty() ? -1 : visibleCache_.last().blockNumber();
            if (lastBlock.isVisible() && lastBlock.blockNumber() > lastCachedNum) {
                visibleCache_.append(lastBlock);
                cachedBlockCount_ = curCount;
                cacheDirty_ = false;
                return;
            } else if (!lastBlock.isVisible()) {
                cachedBlockCount_ = curCount;
                cacheDirty_ = false;
                return;
            }
        }
    }
    visibleCache_.clear();
    visibleCache_.reserve(curCount);
    for (auto b = doc->firstBlock(); b.isValid(); b = b.next()) {
        if (b.isVisible()) {
            visibleCache_.append(b);
        }
    }
    cachedBlockCount_ = curCount;
    cacheDirty_ = false;
}

int Minimap::visibleLineCount() const {
    if (!isVisible() || !qpart_ || !qpart_->document() || qpart_->document()->blockCount() > 20000) {
        return 0;
    }
    ensureCache();
    return visibleCache_.size();
}

int Minimap::visibleViewportStartIndex() const {
    if (!isVisible() || !qpart_ || !qpart_->document() || qpart_->document()->blockCount() > 20000) {
        return 0;
    }
    ensureCache();
    if (visibleCache_.isEmpty()) {
        return 0;
    }
    auto first = qpart_->firstVisibleBlock();
    if (!first.isValid()) {
        return 0;
    }
    for (auto i = 0; i < visibleCache_.size(); ++i) {
        if (visibleCache_.at(i).blockNumber() == first.blockNumber()) {
            return i;
        }
        if (visibleCache_.at(i).blockNumber() > first.blockNumber()) {
            return i;
        }
    }
    return qMax(0, int(visibleCache_.size()) - 1);
}

int Minimap::blockNumberForVisibleIndex(int idx) const {
    ensureCache();
    if (idx < 0 || idx >= visibleCache_.size()) {
        return -1;
    }
    return visibleCache_.at(idx).blockNumber();
}

int Minimap::visibleIndexForBlock(int blockNumber) const {
    ensureCache();
    for (auto i = 0; i < visibleCache_.size(); ++i) {
        if (visibleCache_.at(i).blockNumber() == blockNumber) {
            return i;
        }
    }
    return -1;
}

int Minimap::viewportLineCount() const {
    auto h = qpart_->fontMetrics().height();
    if (h <= 0) {
        return 1;
    }
    return qMax(1, qpart_->viewport()->height() / h);
}

int Minimap::minimapOffsetForStart(int startIndex) const {
    auto total = visibleLineCount();
    auto viewportLines = viewportLineCount();
    auto contentH = total * lineHeight;
    auto visibleH = height();
    if (contentH <= visibleH) {
        return 0;
    }
    auto maxStart = qMax(0, total - viewportLines);
    if (maxStart == 0) {
        return 0;
    }
    auto maxOffset = contentH - visibleH;
    auto offset = qRound(double(startIndex) * maxOffset / maxStart);
    return qBound(0, offset, maxOffset);
}

QRect Minimap::viewportRect() const {
    if (!qpart_) {
        return QRect();
    }
    auto total = visibleLineCount();
    auto viewportLines = viewportLineCount();
    auto start = visibleViewportStartIndex();
    auto viewportHeight = viewportLines * lineHeight;
    auto minimapArea = rect();
    auto minimapVisibleHeight = minimapArea.height();
    auto maxY = qMax(0, minimapVisibleHeight - viewportHeight);
    auto h = qMin(viewportHeight, minimapVisibleHeight);
    auto viewportStartY = 0;
    if (total * lineHeight <= minimapVisibleHeight) {
        maxY = qMax(0, total * lineHeight - viewportHeight);
        viewportStartY = start * lineHeight;
    } else {
        auto maxStart = qMax(1, total - viewportLines);
        viewportStartY = qRound(double(start) * maxY / maxStart);
    }
    viewportStartY = qBound(0, viewportStartY, maxY);
    if (total * lineHeight <= minimapVisibleHeight) {
        h = qMin(h, total * lineHeight);
        h = qMax(lineHeight, h);
    }
    return QRect(minimapArea.left(), viewportStartY, minimapArea.width(), h);
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
    auto font = this->font();
    font.setPointSizeF(2);
    return font;
}

void Minimap::updateScroll(const QPoint &pos) {
    if (!qpart_) {
        return;
    }
    auto total = visibleLineCount();
    if (total == 0) {
        return;
    }
    auto viewportLines = viewportLineCount();
    auto viewportHeight = viewportLines * lineHeight;
    auto visibleH = height();

    // For small docs the drawn text does not fill the widget, so the
    // draggable/clickable range must follow the text extent, not the full
    // widget height. Otherwise reaching the end of the document would
    // require dragging into the empty area below the text.
    auto contentH = total * lineHeight;
    auto travelH = qMin(visibleH, contentH);

    auto clickedVisibleIndex = 0;
    if (travelH > 0) {
        clickedVisibleIndex = qRound(double(pos.y()) * total / travelH);
    }
    clickedVisibleIndex = qBound(0, clickedVisibleIndex, total - 1);

    auto maxStart = qMax(0, total - viewportLines);
    auto targetVisibleIndex = 0;
    if (dragCenter) {
        targetVisibleIndex = qBound(0, clickedVisibleIndex, maxStart);
    } else {
        auto maxViewportY = qMax(0, travelH - viewportHeight);
        auto desiredY = pos.y() - dragOffset;
        desiredY = qBound(0, desiredY, maxViewportY);
        if (maxViewportY > 0) {
            targetVisibleIndex = qRound(double(desiredY) * maxStart / maxViewportY);
        } else {
            targetVisibleIndex = 0;
        }
        targetVisibleIndex = qBound(0, targetVisibleIndex, maxStart);
    }

    // Map visible index -> document block via cache O(1)
    ensureCache();
    auto targetBlock = QTextBlock();
    if (targetVisibleIndex >= 0 && targetVisibleIndex < visibleCache_.size()) {
        targetBlock = visibleCache_.at(targetVisibleIndex);
    } else if (!visibleCache_.isEmpty()) {
        targetBlock = visibleCache_.last();
    }
    if (!targetBlock.isValid()) {
        return;
    }

    auto clickedBlock = QTextBlock();
    if (clickedVisibleIndex >= 0 && clickedVisibleIndex < visibleCache_.size()) {
        clickedBlock = visibleCache_.at(clickedVisibleIndex);
    } else if (!visibleCache_.isEmpty()) {
        clickedBlock = visibleCache_.last();
    }

    qpart_->verticalScrollBar()->setValue(targetBlock.blockNumber());
    // Move the current line to the block the user is clicking or dragging to.
    // setTextCursor() may scroll the viewport to keep the cursor visible, so
    // restore the scroll position chosen by the click/drag math afterwards.
    auto cursorBlock = clickedBlock.isValid() ? clickedBlock : targetBlock;
    if (cursorBlock.isValid()) {
        qpart_->setTextCursor(QTextCursor(cursorBlock));
        qpart_->verticalScrollBar()->setValue(targetBlock.blockNumber());
    }
}

void Minimap::drawMinimapText(QPainter *painter, bool simple) {
    if (!qpart_) {
        return;
    }
    auto minimapArea = rect();
    auto visibleViewportStartLine = visibleViewportStartIndex();
    auto currentLineNumber = qpart_->textCursor().blockNumber();

    auto minimapOffset = minimapOffsetForStart(visibleViewportStartLine);
    auto viewportRect = this->viewportRect();

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
    painter->setFont(minimapFont());

    ensureCache();
    auto total = visibleCache_.size();
    if (total == 0) {
        painter->restore();
        return;
    }
    // Precompute selected blocks for fast per-line check during mouse selection drag
    auto selectedBlocks = QSet<int>();
    {
        auto docForSel = qpart_->document();
        auto addSelection = [&](const QTextCursor &c) {
            if (!c.hasSelection()) return;
            auto s = static_cast<int>(c.selectionStart());
            auto e = static_cast<int>(c.selectionEnd());
            if (e <= s) return;
            auto startBlock = docForSel->findBlock(s);
            auto endBlock = docForSel->findBlock(e);
            for (auto b = startBlock; b.isValid(); b = b.next()) {
                if (!b.isVisible()) {
                    if (b == endBlock) break;
                    continue;
                }
                auto bStart = static_cast<int>(b.position());
                auto bLen = static_cast<int>(b.text().length());
                if (bLen == 0) {
                    if (b == endBlock) break;
                    continue;
                }
                auto bEnd = bStart + bLen;
                auto is = std::max(bStart, s);
                auto ie = std::min(bEnd, e);
                if (ie > is) {
                    auto ratio = double(ie - is) / bLen;
                    if (ratio >= 0.5) {
                        selectedBlocks.insert(b.blockNumber());
                    }
                }
                if (b == endBlock) break;
            }
        };
        addSelection(qpart_->textCursor());
        for (auto &ec : qpart_->extraCursors) {
            addSelection(ec);
        }
    }
    auto startDrawIdx = minimapOffset / lineHeight;
    startDrawIdx = qBound(0, startDrawIdx, qMax(0, total - 1));
    auto maxLines = minimapArea.height() / lineHeight + 3;
    auto endIdx = qMin(total, startDrawIdx + maxLines);
    for (auto idx = startDrawIdx; idx < endIdx; ++idx) {
        auto y = idx * lineHeight - minimapOffset;
        if (y >= minimapArea.height()) {
            break;
        }
        if (y + lineHeight < 0) {
            continue;
        }
        auto curBlock = visibleCache_.at(idx);
        if (!curBlock.isValid()) {
            continue;
        }
        auto blockNumber = curBlock.blockNumber();
        auto backgronud = QColor(Qt::transparent);
        int flags[] = {BOOMARK_BIT, MODIFIED_BIT,   WARNING_BIT,  ERROR_BIT,
                       INFO_BIT,    BREAKPOINT_BIT, EXECUTING_BIT};
        if (blockNumber == currentLineNumber) {
            backgronud = qpart_->currentLineColor();
        }
        for (auto flag : flags) {
            if (hasFlag(curBlock, flag)) {
                auto color = qpart_->getColorForLineFlag(flag);
                if (color.alpha() != 0) {
                    backgronud = blendColors(color, backgronud);
                }
            }
        }
        auto isSelected = selectedBlocks.contains(blockNumber);
        auto selColor = QColor();
        if (isSelected) {
            selColor = qpart_->palette().color(QPalette::Highlight);
            selColor.setAlpha(130);
        }
        if (backgronud.alpha() != 0) {
            painter->setPen(Qt::NoPen);
            painter->setBrush(backgronud);
            painter->drawRect(minimapArea.left(), y, minimapArea.width(), lineHeight);
        }
        if (isSelected) {
            painter->setPen(Qt::NoPen);
            painter->setBrush(selColor);
            if (simple) {
                auto textLen = curBlock.text().length();
                auto selW = qMin(minimapArea.width(), textLen * charWidth);
                painter->drawRect(minimapArea.left(), y, selW, lineHeight);
            } else {
                auto fm = painter->fontMetrics();
                auto textW = fm.horizontalAdvance(curBlock.text());
                auto availW = minimapArea.width() - 10;
                auto selW = qMin(availW, textW);
                painter->drawRect(minimapArea.left() + 5, y, selW, lineHeight);
            }
        }
        painter->setPen(textColor);
        if (simple) {
            auto lineText = curBlock.text();
            for (auto charIndex = 0; charIndex < lineText.length(); ++charIndex) {
                auto dotX = minimapArea.left() + charIndex * charWidth;
                if (dotX >= minimapArea.right()) {
                    break;
                }
                auto isDrawable = lineText.at(charIndex).isLetterOrNumber() ||
                                  lineText.at(charIndex).isPunct();
                if (isDrawable) {
                    painter->drawPoint(dotX, y);
                }
            }
        } else {
            auto padding = 5;
            auto textRect = QRectF(minimapArea.left() + padding, y,
                                   minimapArea.width() - padding * 2, lineHeight);
            painter->drawText(textRect, Qt::AlignLeft, curBlock.text());
        }
    }

    painter->restore();
}

FoldingArea::FoldingArea(Qutepart *editor) : SideArea(editor) { setMouseTracking(true); }

int FoldingArea::widthHint() const { return qpart_->fontMetrics().height(); }

void FoldingArea::paintEvent(QPaintEvent *event) {
    auto palette = qpart_->palette();
    auto background = palette.color(QPalette::AlternateBase);
    auto textColor = palette.color(QPalette::Text);
    textColor.setAlpha(85);

    if (auto theme = qpart_->getTheme()) {
        if (theme->getEditorColors().contains(Theme::Colors::IconBorder)) {
            background = theme->getEditorColors()[Theme::Colors::IconBorder];
        }
    }

    QPainter painter(this);
    painter.fillRect(event->rect(), background);

    auto block = qpart_->firstVisibleBlock();
    auto top = qRound(qpart_->blockBoundingRect(block).translated(qpart_->contentOffset()).top());
    auto bottom = top + qRound(qpart_->blockBoundingRect(block).height());
    while (block.isValid() && top <= event->rect().bottom()) {
        if (block.isVisible() && bottom >= event->rect().top()) {
            auto blockData = static_cast<TextBlockUserData *>(block.userData());
            auto prevBlock = block.previous();
            auto prevLevel = 0;
            TextBlockUserData *prevData = nullptr;
            if (prevBlock.isValid()) {
                prevData = static_cast<TextBlockUserData *>(prevBlock.userData());
            }

            if (prevData) {
                prevLevel = prevData->folding.level;
            }

            // Debug: print folding level for non-foldable lines
            if (m_debugFolding) {
                auto r = QRect(1, top + 1, width() - 2, qpart_->fontMetrics().height() - 2);
                painter.setPen(textColor);
                painter.drawText(r, Qt::AlignCenter,
                                 QString::number(blockData ? blockData->folding.level : 0));
            } else {
                if (blockData && blockData->folding.level > prevLevel) {
                    auto symbol = block.next().isVisible() ? "-" : "+";
                    auto lineHeight = (int)qpart_->blockBoundingRect(block).height();
                    auto lineRect = QRect(1, top, width() - 2, lineHeight);
                    auto side = qMin(lineRect.width(), lineRect.height());
                    auto squareRect = QRect(lineRect.x() + (lineRect.width() - 2 - side) / 2,
                                            lineRect.y() + (lineRect.height() - 2 - side) / 2,
                                            side - 1, side - 1);

                    painter.setPen(textColor);
                    painter.drawRect(squareRect);
                    painter.drawText(squareRect, Qt::AlignCenter, symbol);
                }
            };
        }

        block = block.next();
        top = bottom;
        bottom = top + qRound(qpart_->blockBoundingRect(block).height());
    }
}

QTextBlock FoldingArea::blockAt(const QPoint &pos) const {
    QTextBlock block = qpart_->firstVisibleBlock();
    if (!block.isValid()) {
        return QTextBlock();
    }

    auto top = qRound(qpart_->blockBoundingRect(block).translated(qpart_->contentOffset()).top());
    auto bottom = top + qRound(qpart_->blockBoundingRect(block).height());
    while (block.isValid() && top <= pos.y()) {
        if (block.isVisible() && bottom >= pos.y()) {
            return block;
        }
        block = block.next();
        top = bottom;
        bottom = top + qRound(qpart_->blockBoundingRect(block).height());
    }

    return QTextBlock();
}

void FoldingArea::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        auto textBlock = blockAt(event->pos());
        if (textBlock.isValid()) {
            auto const *blockData = static_cast<TextBlockUserData *>(textBlock.userData());
            auto prevBlock = textBlock.previous();
            auto prevLevel = 0;
            if (prevBlock.isValid()) {
                auto const *prevData = static_cast<TextBlockUserData *>(prevBlock.userData());
                if (prevData) {
                    prevLevel = prevData->folding.level;
                }
            }

            if (blockData && blockData->folding.level > prevLevel) {
                emit foldClicked(textBlock.blockNumber());
                event->accept();
                return;
            }
        }
    }
    QWidget::mousePressEvent(event);
}

} // namespace Qutepart

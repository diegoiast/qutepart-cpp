/*
 * Copyright (C) 2018-2023 Andrei Kopats
 * Copyright (C) 2023-...  Diego Iastrubni <diegoiast@gmail.com>
 * SPDX-License-Identifier: MIT
 */

#include <QAction>
#include <QApplication>
#include <QClipboard>
#include <QDebug>
#include <QKeyEvent>
#include <QPainter>
#include <QScrollBar>
#include <QStyle>

#include "bracket_highlighter.h"
#include "completer.h"
#include "qutepart.h"
#include "side_areas.h"
#include "text_block_flags.h"
#include "text_block_utils.h"

#include "hl/syntax_highlighter.h"
#include "hl_factory.h"
#include "indent/indent_funcs.h"
#include "indent/indenter.h"
#include "theme.h"

namespace Qutepart {

Qutepart::Qutepart(QWidget *parent, const QString &text)
    : QPlainTextEdit(text, parent), indenter_(new Indenter(this)), markArea_(new MarkArea(this)),
      completer_(new Completer(this)), foldingArea_(new FoldingArea(this)), drawIndentations_(true),
      drawAnyWhitespace_(false), drawIncorrectIndentation_(true), drawSolidEdge_(true),
      enableSmartHomeEnd_(true), softLineWrapping_(true), smartFolding_(true), lineLengthEdge_(80),
      brakcetsQutoEnclose(true), completionEnabled_(true), completionThreshold_(3),
      viewportMarginStart_(0) {
    extraCursorBlinkTimer_ = new QTimer(this);
    setBracketHighlightingEnabled(true);
    setLineNumbersVisible(true);
    setMinimapVisible(true);
    setMarkCurrentWord(true);
    foldingArea_->show();
    connect(foldingArea_, &FoldingArea::foldClicked, this, &Qutepart::toggleFold);
    setDrawSolidEdge(drawSolidEdge_);

    setDefaultColors();
    initActions();
    setAttribute(Qt::WA_KeyCompression, false); // vim can't process compressed keys
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);

    updateTabStopWidth();
    connect(this, &Qutepart::cursorPositionChanged, this, [this]() {
        lastWordUnderCursor.clear();
        updateExtraSelections();
        viewport()->update();
        if (miniMap_) {
            miniMap_->update();
        }        
    });

    connect(document(), &QTextDocument::contentsChange, this, [this]() {
        auto block = textCursor().block();
        this->setLineModified(block, true);

        // Event is fired when destructing as well
        if (markArea_) {
            markArea_->update();
        }
    });

    QTimer::singleShot(0, this, [this]() { updateViewport(); });

    auto palette = style()->standardPalette();
    auto c = palette.brush(QPalette::Highlight).color();
    c.setAlpha(180);
    palette.setBrush(QPalette::Highlight, c);
    palette.setBrush(QPalette::HighlightedText, QBrush(Qt::NoBrush));
    setPalette(palette);

    // Setup extra cursor blinking timer
    extraCursorBlinkTimer_->setInterval(QApplication::cursorFlashTime() / 2);
    connect(extraCursorBlinkTimer_, &QTimer::timeout, this,
            &Qutepart::toggleExtraCursorsVisibility);
}

QList<QTextEdit::ExtraSelection> Qutepart::highlightText(const QString &text, bool fullWords) {
    if (blockCount() > MaxLinesForWordHighligher) {
        return {};
    }
    auto cursor = QTextCursor(document());
    auto extraSelections = QList<QTextEdit::ExtraSelection>();
    auto format = QTextCharFormat();
    auto palette = style()->standardPalette();
    auto color = palette.color(QPalette::Highlight);
    color.setAlphaF(0.1f);

    if (theme) {
        color = theme->getEditorColors()[Theme::Colors::SearchHighlight];
    }
    format.setBackground(color);
    while (!cursor.isNull() && !cursor.atEnd()) {
        QTextDocument::FindFlags flags;
        if (fullWords) {
            flags = QTextDocument::FindWholeWords;
        } else {
            // TODO: do this only if the current language is case insensitive (for example: pascal)
            flags = QTextDocument::FindCaseSensitively;
        }
        cursor = document()->find(text, cursor, flags);
        if (!cursor.isNull()) {
            QTextEdit::ExtraSelection extra;
            extra.format = format;
            extra.cursor = cursor;
            extraSelections.append(extra);
        }
    }
    if (extraSelections.length() < 2) {
        return {};
    }
    return extraSelections;
}

Qutepart::~Qutepart() {}

Lines Qutepart::lines() const { return Lines(document()); }

void Qutepart::setHighlighter(const QString &languageId) {
    auto hl = static_cast<SyntaxHighlighter *>(highlighter_);
    if (hl) {
        auto name = hl->getLanguage()->fileName;
        if (name == languageId) {
            return;
        }
    }
    indenter_->setLanguage(languageId);
    highlighter_ = makeHighlighter(document(), languageId);
    hl = static_cast<SyntaxHighlighter *>(highlighter_);
    if (hl) {
        auto lang = hl->getLanguage();
        completer_->setKeywords(lang->allLanguageKeywords());
        hl->setTheme(theme);
    } else {
        completer_->setKeywords({});
    }
}

void Qutepart::removeHighlighter() {
    delete highlighter_;
    highlighter_ = nullptr;
    completer_->setKeywords({});
}

void Qutepart::setIndentAlgorithm(IndentAlg indentAlg) { indenter_->setAlgorithm(indentAlg); }

void Qutepart::setDefaultColors() {
    QPalette palette = style()->standardPalette();
    currentLineColor_ = palette.color(QPalette::Highlight);
    currentLineColor_.setAlphaF(0.2f);
    whitespaceColor_ = palette.color(QPalette::Text);
    whitespaceColor_.setAlphaF(0.1f);
    lineLengthEdgeColor_ = palette.color(QPalette::Accent);
    lineLengthEdgeColor_.setAlphaF(0.5f);
    lineNumberColor = palette.color(QPalette::Text);
    currentLineNumberColor = palette.color(QPalette::ButtonText);
    indentColor_ = whitespaceColor_;
}

void Qutepart::setTheme(const Theme *newTheme) {
    auto hl = dynamic_cast<SyntaxHighlighter *>(highlighter_);
    theme = newTheme;
    if (hl) {
        hl->setTheme(theme);
        hl->rehighlight();
    }

    fixLineFlagColors();
    if (!newTheme) {
        setDefaultColors();

        auto palette = style()->standardPalette();
        auto c = palette.brush(QPalette::Highlight).color();
        c.setAlpha(180);
        palette.setBrush(QPalette::Highlight, c);
        palette.setBrush(QPalette::HighlightedText, QBrush(Qt::NoBrush));
        setPalette(palette);

        update();
        updateExtraSelections();
        return;
    }

    lineNumberColor = theme->getEditorColors()[Theme::Colors::LineNumbers];
    currentLineNumberColor = theme->getEditorColors()[Theme::Colors::CurrentLineNumber];

    lineLengthEdgeColor_ = theme->getEditorColors()[Theme::Colors::WordWrapMarker];
    currentLineColor_ = theme->getEditorColors()[Theme::Colors::CurrentLine];
    indentColor_ = theme->getEditorColors()[Theme::Colors::IndentationLine];
    whitespaceColor_ = theme->getEditorColors()[Theme::Colors::IndentationLine];

    // QPalette p(palette());
    auto palette = style()->standardPalette();
    if (theme->getEditorColors().contains(Theme::Colors::BackgroundColor) &&
        theme->getEditorColors()[Theme::Colors::BackgroundColor].isValid()) {
        palette.setColor(QPalette::Base, theme->getEditorColors()[Theme::Colors::BackgroundColor]);
        palette.setColor(QPalette::Text,
                         theme->getTextStyles()[QStringLiteral("Normal")]["text-color"]);
    }

    if (theme->getEditorColors().contains(Theme::Colors::TextSelection) &&
        theme->getEditorColors()[Theme::Colors::TextSelection].isValid()) {
        auto c = theme->getEditorColors()[Theme::Colors::TextSelection];
        palette.setBrush(QPalette::Highlight, c);
        palette.setBrush(QPalette::HighlightedText, QBrush(Qt::NoBrush));
    }

    setPalette(palette);

    updateExtraSelections();
}

TextCursorPosition Qutepart::textCursorPosition() const {
    QTextCursor cursor = textCursor();
    return TextCursorPosition{cursor.blockNumber(), cursor.positionInBlock()};
}

void Qutepart::goTo(int line, int column) {
    QTextBlock block = document()->findBlockByNumber(line);
    QTextCursor cursor(block);

    if (column != 0) {
        if (column < 0) {
            column = 0;
        } else if (column > cursor.block().length()) {
            column = cursor.block().length() - 1;
        }
        cursor.setPosition(cursor.position() + column);
    }

    setTextCursor(cursor);
    updateExtraSelections();
}

void Qutepart::goTo(const TextCursorPosition &pos) { return goTo(pos.line, pos.column); }

void Qutepart::autoIndentCurrentLine() {
    QTextCursor cursor = textCursor();
    indenter_->indentBlock(cursor.block(), 0, QChar::Null);
}

bool Qutepart::indentUseTabs() const { return indenter_->useTabs(); }

void Qutepart::setIndentUseTabs(bool use) { indenter_->setUseTabs(use); }

int Qutepart::indentWidth() const { return indenter_->width(); }

void Qutepart::setIndentWidth(int width) {
    indenter_->setWidth(width);
    updateTabStopWidth();
}

bool Qutepart::drawIndentations() const { return drawIndentations_; }

void Qutepart::setDrawIndentations(bool draw) { drawIndentations_ = draw; }

bool Qutepart::drawAnyWhitespace() const { return drawAnyWhitespace_; }

void Qutepart::setDrawAnyWhitespace(bool draw) { drawAnyWhitespace_ = draw; }

bool Qutepart::drawIncorrectIndentation() const { return drawIncorrectIndentation_; }

void Qutepart::setDrawIncorrectIndentation(bool draw) { drawIncorrectIndentation_ = draw; }

bool Qutepart::drawSolidEdge() const { return drawSolidEdge_; }

void Qutepart::setDrawSolidEdge(bool draw) {
    drawSolidEdge_ = draw;
    update();
}

bool Qutepart::softLineWrapping() const { return softLineWrapping_; }

void Qutepart::setSoftLineWrapping(bool enable) { softLineWrapping_ = enable; }

bool Qutepart::smartFolding() const { return smartFolding_; }

void Qutepart::setSmartFolding(bool enabled) { smartFolding_ = enabled; }

int Qutepart::lineLengthEdge() const { return lineLengthEdge_; }

void Qutepart::setLineLengthEdge(int edge) { lineLengthEdge_ = edge; }

QColor Qutepart::lineLengthEdgeColor() const { return lineLengthEdgeColor_; }

void Qutepart::setLineLengthEdgeColor(QColor color) { lineLengthEdgeColor_ = color; }

QColor Qutepart::currentLineColor() const { return currentLineColor_; }

void Qutepart::setCurrentLineColor(QColor color) { currentLineColor_ = color; }

bool Qutepart::bracketHighlightingEnabled() const { return bracketHighlighter_ != nullptr; }

void Qutepart::setBracketHighlightingEnabled(bool value) {
    if (value && (!bracketHighlighter_)) {
        bracketHighlighter_ = new BracketHighlighter(this);
    } else if ((!value) && bool(bracketHighlighter_)) {
        delete bracketHighlighter_;
        bracketHighlighter_ = nullptr;
    }
    updateExtraSelections();
}

bool Qutepart::lineNumbersVisible() const { return bool(lineNumberArea_); }

void Qutepart::setLineNumbersVisible(bool value) {
    if (!value && lineNumberArea_) {
        delete lineNumberArea_;
        lineNumberArea_ = nullptr;
    } else if (value && (!lineNumberArea_)) {
        lineNumberArea_ = new LineNumberArea(this);
        connect(lineNumberArea_, &LineNumberArea::widthChanged, this, &Qutepart::updateViewport);
    }
    updateViewport();
}

bool Qutepart::minimapVisible() const { return lineNumberArea_ != nullptr; }

void Qutepart::setMinimapVisible(bool value) {
    if ((miniMap_ != nullptr) == value) {
        return;
    }

    if (miniMap_) {
        delete miniMap_;
        miniMap_ = nullptr;
    } else {
        miniMap_ = new Minimap(this);
        miniMap_->show();
    }
    updateViewport();
}

bool Qutepart::getSmartHomeEnd() const { return enableSmartHomeEnd_; }

void Qutepart::setSmartHomeEnd(bool value) { enableSmartHomeEnd_ = value; }

void Qutepart::setMarkCurrentWord(bool enable) {
    if (!enable) {
        delete currentWordTimer;
        currentWordTimer = nullptr;
        lastWordUnderCursor.clear();
        updateExtraSelections();
        return;
    }

    currentWordTimer = new QTimer(this);
    currentWordTimer->setSingleShot(true);
    currentWordTimer->setInterval(500);

    connect(currentWordTimer, &QTimer::timeout, currentWordTimer, [this]() {
        if (lastWordUnderCursor.length() > 2) {
            updateExtraSelections();
        }
    });

    connect(this, &QPlainTextEdit::cursorPositionChanged, currentWordTimer, [this]() {
        auto cursor = textCursor();
        cursor.select(QTextCursor::WordUnderCursor);
        auto wordUnderCursor = cursor.selectedText();

        lastWordUnderCursor.clear();
        if (currentWordTimer) {
            currentWordTimer->stop();
        }
        updateExtraSelections();
        if (wordUnderCursor.isEmpty() || wordUnderCursor.length() <= 2) {
            return;
        }
        lastWordUnderCursor = wordUnderCursor;
        if (currentWordTimer) {
            currentWordTimer->start();
        }
    });

    auto cursor = textCursor();
    cursor.select(QTextCursor::WordUnderCursor);
    auto wordUnderCursor = cursor.selectedText();
    lastWordUnderCursor = wordUnderCursor;

    updateExtraSelections();
}

bool Qutepart::getMarkCurrentWord() const { return currentWordTimer != nullptr; }

void Qutepart::setBracketAutoEnclose(bool enable) { brakcetsQutoEnclose = enable; }

bool Qutepart::getBracketAutoEnclose() const { return brakcetsQutoEnclose; }

bool Qutepart::completionEnabled() const { return completionEnabled_; }

void Qutepart::setCompletionEnabled(bool val) { completionEnabled_ = val; }

int Qutepart::completionThreshold() const { return completionThreshold_; }

/// Returns the status of a line. A line is marked as modified when its changed via the user
bool Qutepart::isLineModified(int lineNumber) const {
    auto block = document()->findBlockByNumber(lineNumber);
    if (!block.isValid()) {
        return false;
    }
    return hasFlag(block, MODIFIED_BIT);
}

/// Set the status of a line, modified or not
void Qutepart::setLineModified(int lineNumber, bool modified) const {
    auto block = document()->findBlockByNumber(lineNumber);
    setLineModified(block, modified);
}

/// Set the status of a line, modified or not
void Qutepart::setLineModified(QTextBlock &block, bool modified) const {
    setFlag(block, MODIFIED_BIT, modified);
    if (markArea_) {
        markArea_->update();
    }
}

/// Clear modifications from all document.
void Qutepart::removeModifications() {
    for (auto block = document()->begin(); block != document()->end(); block = block.next()) {
        setFlag(block, MODIFIED_BIT, false);
    }
}

// Markings
void Qutepart::modifyBlockFlag(int lineNumber, int bit, bool status, QColor background) {
    auto block = document()->findBlockByNumber(lineNumber);
    if (!block.isValid()) {
        qDebug() << "Invalid line " << lineNumber << "cannot set status" << bit;
        return;
    }
    setFlag(block, bit, status);

    if (background != Qt::transparent) {
        QTextEdit::ExtraSelection selection;
        selection.format.setBackground(background);
        selection.format.setProperty(QTextFormat::FullWidthSelection, true);
        selection.cursor = QTextCursor(block);
        selection.cursor.clearSelection();
        persitentSelections.append(selection);
        updateExtraSelections();
    }
    if (markArea_) {
        markArea_->update();
    }
    if (miniMap_) {
        miniMap_->update();
    }
}

bool Qutepart::getBlockFlag(int lineNumber, int bit) const {
    auto block = document()->findBlockByNumber(lineNumber);
    return hasFlag(block, bit);
}

bool Qutepart::getLineBookmark(int lineNumber) const {
    return getBlockFlag(lineNumber, BOOMARK_BIT);
}

void Qutepart::setLineBookmark(int lineNumber, bool status) {
    modifyBlockFlag(lineNumber, BOOMARK_BIT, status, Qt::transparent);
}

bool Qutepart::getLineWarning(int lineNumber) const {
    return getBlockFlag(lineNumber, WARNING_BIT);
}

void Qutepart::setLineWarning(int lineNumber, bool status) {
    modifyBlockFlag(lineNumber, WARNING_BIT, status, getColorForLineFlag(WARNING_BIT));
}

bool Qutepart::getLineError(int lineNumber) const { return getBlockFlag(lineNumber, ERROR_BIT); }

void Qutepart::setLineError(int lineNumber, bool status) {
    modifyBlockFlag(lineNumber, ERROR_BIT, status, getColorForLineFlag(ERROR_BIT));
}

bool Qutepart::getLineInfo(int lineNumber) const { return getBlockFlag(lineNumber, INFO_BIT); }

void Qutepart::setLineInfo(int lineNumber, bool status) {
    modifyBlockFlag(lineNumber, INFO_BIT, status, getColorForLineFlag(INFO_BIT));
}

bool Qutepart::getLineBreakpoint(int lineNumber) const {
    return getBlockFlag(lineNumber, BREAKPOINT_BIT);
}

void Qutepart::setLineBreakpoint(int lineNumber, bool status) {
    modifyBlockFlag(lineNumber, BREAKPOINT_BIT, status, getColorForLineFlag(BREAKPOINT_BIT));
}

bool Qutepart::getLineExecuting(int lineNumber) const {
    return getBlockFlag(lineNumber, EXECUTING_BIT);
}

void Qutepart::setLineExecuting(int lineNumber, bool status) {
    modifyBlockFlag(lineNumber, EXECUTING_BIT, status, getColorForLineFlag(EXECUTING_BIT));
}

void Qutepart::removeMetaData() {
    for (auto block = document()->begin(); block != document()->end(); block = block.next()) {
        auto data = static_cast<TextBlockUserData *>(block.userData());
        if (data) {
            data->metaData.message.clear();
            data->state = 0;
        }
    }
    persitentSelections.clear();
}

void Qutepart::setLineMessage(int lineNumber, const QString &message) {
    auto block = document()->findBlockByNumber(lineNumber);
    auto data = static_cast<TextBlockUserData *>(block.userData());
    if (!data) {
        data = new TextBlockUserData({}, {nullptr});
        block.setUserData(data);
    }
    data->metaData.message = message;
}

auto Qutepart::getColorForLineFlag(int flag) -> QColor {
    // https://www.color-hex.com/color-palette/5361
    auto color = QColor(Qt::transparent);
    switch (flag) {
    case INFO_BIT:
        color = QColor(0xbae1ff);
        break;
    case WARNING_BIT:
        if (theme && theme->getEditorColors().contains(Theme::Colors::MarkWarning)) {
            color = theme->getEditorColors().value(Theme::Colors::MarkWarning);
        } else {
            color = QColor(0xffffba);
        }
        break;
    case ERROR_BIT:
        if (theme && theme->getEditorColors().contains(Theme::Colors::MarkError)) {
            color = theme->getEditorColors().value(Theme::Colors::MarkError);
        } else {
            color = QColor(0xffb3ba);
        }
        break;

    // not tested yet
    case EXECUTING_BIT:
        if (theme && theme->getEditorColors().contains(Theme::Colors::MarkExecution)) {
            color = theme->getEditorColors().value(Theme::Colors::MarkExecution);
        } else {
            color = QColor(Qt::blue);
        }
        break;
    case BREAKPOINT_BIT:
        if (theme && theme->getEditorColors().contains(Theme::Colors::MarkBreakpointActive)) {
            color = theme->getEditorColors().value(Theme::Colors::MarkBreakpointActive);
        } else {
            color = QColor(Qt::magenta);
        }
        break;
    default:
        break;
    }
    return color;
}

auto Qutepart::fixLineFlagColors() -> void {
    persitentSelections.clear();

    auto block = document()->firstBlock();
    while (block.isValid()) {
        QTextEdit::ExtraSelection selection;
        QTextCursor cursor(block);
        cursor.clearSelection();
        selection.cursor = cursor;
        selection.format.setProperty(QTextFormat::FullWidthSelection, true);

        int flags[] = {BOOMARK_BIT, MODIFIED_BIT,   WARNING_BIT,  ERROR_BIT,
                       INFO_BIT,    BREAKPOINT_BIT, EXECUTING_BIT};
        for (int flag : flags) {
            if (hasFlag(block, flag)) {
                auto color = getColorForLineFlag(flag);
                if (color != Qt::transparent) {
                    selection.format.setBackground(color);
                    persitentSelections.append(selection);
                }
            }
        }
        block = block.next();
    }
    updateExtraSelections();
}

void Qutepart::setCompletionThreshold(int val) { completionThreshold_ = val; }

void Qutepart::resetSelection() {
    QTextCursor cursor = textCursor();
    cursor.setPosition(cursor.position());
    setTextCursor(cursor);
}

namespace {
// Check if an event may be a typed character
static bool isCharEvent(QKeyEvent *ev) {
    QString text = ev->text();
    if (text.length() != 1) {
        return false;
    }

    if (ev->modifiers() != Qt::ShiftModifier && ev->modifiers() != Qt::KeypadModifier &&
        ev->modifiers() != Qt::NoModifier) {
        return false;
    }

    QChar code = text[0];
    if (code <= QChar(31) || code == QChar(0x7f)) { // control characters
        return false;
    }

    if (code == ' ' && ev->modifiers() == Qt::ShiftModifier) {
        return false; // Shift+Space is a shortcut, not a text
    }

    return true;
}

static void addBrackets(QTextCursor &cursor, QChar openBracket, QChar closeBracket) {
    auto start = cursor.selectionStart();
    auto end = cursor.selectionEnd();
    cursor.beginEditBlock();
    cursor.setPosition(start);
    cursor.insertText(QString(openBracket));
    cursor.setPosition(end + 1);
    cursor.insertText(QString(closeBracket));
    cursor.endEditBlock();
}

} // anonymous namespace

QVector<int> Qutepart::getFoldedLines() const {
    QVector<int> foldedLines;
    for (auto block = document()->begin(); block != document()->end(); block = block.next()) {
        auto data = static_cast<TextBlockUserData *>(block.userData());
        if (data && data->folding.folded) {
            foldedLines << block.blockNumber();
        }
    }
    return foldedLines;
}

void Qutepart::setFoldedLines(const QVector<int> &foldedLines) {
    for (auto block = document()->begin(); block != document()->end(); block = block.next()) {
        auto data = static_cast<TextBlockUserData *>(block.userData());
        if (data) {
            data->folding.folded = false;
        }
    }

    for (auto lineNumber : foldedLines) {
        auto block = document()->findBlockByNumber(lineNumber);
        if (block.isValid()) {
            auto data = static_cast<TextBlockUserData *>(block.userData());
            if (data) {
                data->folding.folded = true;
            }
        }
    }
    viewport()->update();
}

void Qutepart::setBlockFolded(QTextBlock &block, bool folded) {
    if (!block.isValid()) {
        return;
    }

    auto data = static_cast<TextBlockUserData *>(block.userData());
    if (!data) {
        return;
    }

    auto currentFoldLevel = data->folding.level;
    if (currentFoldLevel == 0) {
        return;
    }

    if (data->folding.folded == folded) {
        return;
    }

    auto currentCursor = textCursor();
    auto cursorInFoldedRegion = false;
    if (folded) {
        if (currentCursor.block().blockNumber() > block.blockNumber()) {
            for (auto it = block.next(); it.isValid(); it = it.next()) {
                auto itData = static_cast<TextBlockUserData *>(it.userData());
                if (it == currentCursor.block()) {
                    cursorInFoldedRegion = true;
                    break;
                }
                if (itData && itData->folding.level < currentFoldLevel) {
                    break; // end of region
                }
            }
        }
    }

    data->folding.folded = folded;

    if (folded) {
        for (auto nextBlock = block.next(); nextBlock.isValid(); nextBlock = nextBlock.next()) {
            auto blockData = static_cast<TextBlockUserData *>(nextBlock.userData());
            nextBlock.setVisible(false);
            nextBlock.setLineCount(0);
            if (blockData && blockData->folding.level < currentFoldLevel) {
                break;
            }
        }
    } else {
        for (auto nextBlock = block.next(); nextBlock.isValid(); nextBlock = nextBlock.next()) {
            auto blockData = static_cast<TextBlockUserData *>(nextBlock.userData());
            nextBlock.setVisible(true);
            nextBlock.setLineCount(1);

            if (smartFolding_) {
                if (blockData && blockData->folding.folded) {
                    blockData->folding.folded = false;
                }
            } else if (blockData && blockData->folding.folded) {
                auto innerFoldLevel = blockData->folding.level;
                auto blockToSkip = nextBlock.next();
                while (blockToSkip.isValid()) {
                    auto skipData = static_cast<TextBlockUserData *>(blockToSkip.userData());
                    if (skipData && skipData->folding.level < innerFoldLevel) {
                        nextBlock = blockToSkip.previous();
                        break;
                    }
                    if (!blockToSkip.next().isValid()) {
                        nextBlock = blockToSkip;
                        break;
                    }
                    blockToSkip = blockToSkip.next();
                }
            }

            if (blockData && blockData->folding.level < currentFoldLevel) {
                break;
            }
        }
    }

    if (cursorInFoldedRegion) {
        QTextCursor newCursor(block);
        setTextCursor(newCursor);
    }

    viewport()->update();
    if (foldingArea_) {
        foldingArea_->update();
    }
    if (lineNumberArea_) {
        lineNumberArea_->update();
    }
    if (markArea_) {
        markArea_->update();
    }
    if (miniMap_) {
        miniMap_->update();
    }
}

void Qutepart::foldBlock(int lineNumber) {
    auto block = document()->findBlockByNumber(lineNumber);

    if (smartFolding_) {
        auto data = static_cast<TextBlockUserData *>(block.userData());
        if (data->folding.folded) {
            auto parentBlock = findBlockToFold(block.previous());
            if (parentBlock.isValid() && parentBlock != block) {
                setBlockFolded(parentBlock, true);
                return;
            }
        }
    }

    setBlockFolded(block, true);
}

void Qutepart::unfoldBlock(int lineNumber) {
    auto block = document()->findBlockByNumber(lineNumber);
    setBlockFolded(block, false);
}

void Qutepart::toggleFold(int lineNumber) {
    auto block = document()->findBlockByNumber(lineNumber);
    if (!block.isValid()) {
        return;
    }
    auto data = static_cast<TextBlockUserData *>(block.userData());
    if (!data) {
        return;
    }
    setBlockFolded(block, !data->folding.folded);
}

QTextBlock Qutepart::findBlockToFold(QTextBlock block) {
    auto data = static_cast<TextBlockUserData *>(block.userData());
    if (!data) {
        return QTextBlock();
    }

    auto prev = block.previous();
    if (prev.isValid()) {
        auto prevData = static_cast<TextBlockUserData *>(prev.userData());
        if (prevData && prevData->folding.level > data->folding.level) {
            block = prev;
            data = prevData;
        } else if (data->folding.level == 0) {
            return QTextBlock();
        }
    }

    auto level = data->folding.level;
    auto blockToSearch = block.previous();
    while (blockToSearch.isValid()) {
        auto searchData = static_cast<TextBlockUserData *>(blockToSearch.userData());
        if (searchData && searchData->folding.level < level) {
            return blockToSearch.next();
        }
        blockToSearch = blockToSearch.previous();
    }

    return document()->firstBlock();
}

void Qutepart::foldCurrentBlock() {
    auto blockToFold = findBlockToFold(textCursor().block());
    if (blockToFold.isValid()) {
        foldBlock(blockToFold.blockNumber());
    }
}

void Qutepart::unfoldCurrentBlock() {
    auto cursor = textCursor();
    auto block = cursor.block();

    while (block.isValid()) {
        auto data = static_cast<TextBlockUserData *>(block.userData());
        if (data && data->folding.folded) {
            unfoldBlock(block.blockNumber());
            return;
        }
        block = block.previous();
    }
}

void Qutepart::toggleCurrentFold() {
    QTextBlock blockToFold = findBlockToFold(textCursor().block());
    if (blockToFold.isValid()) {
        auto data = static_cast<TextBlockUserData *>(blockToFold.userData());
        if (data && data->folding.folded) {
            auto parentBlock = findBlockToFold(blockToFold.previous());
            if (parentBlock.isValid() && parentBlock != blockToFold) {
                setBlockFolded(parentBlock, true); // Fold parent
                return;
            }
        }
        toggleFold(blockToFold.blockNumber());
    }
}

void Qutepart::foldTopLevelBlocks() {
    QTextBlock firstLevel1AreaStart;
    auto level1AreaCount = 0;
    auto prevLevel = 0;

    for (auto block = document()->begin(); block != document()->end(); block = block.next()) {
        auto data = static_cast<TextBlockUserData *>(block.userData());
        auto currentLevel = data ? data->folding.level : 0;
        if (currentLevel == 1 && prevLevel == 0) {
            level1AreaCount++;
            if (level1AreaCount == 1) {
                firstLevel1AreaStart = block;
            }
        }
        prevLevel = currentLevel;
    }

    // If we have a single top level folding area, keep it open, and fold only the nested ones
    auto levelsToFold = (level1AreaCount == 1) ? 2 : 1;
    auto block = firstLevel1AreaStart.next();
    while (block.isValid()) {
        auto data = static_cast<TextBlockUserData *>(block.userData());
        if (data) {
            if (data->folding.level == levelsToFold) {
                setBlockFolded(block, true);
            }
        }
        block = block.next();
    }
}

void Qutepart::unfoldAll() {
    for (auto block = document()->begin(); block != document()->end(); block = block.next()) {
        auto data = static_cast<TextBlockUserData *>(block.userData());
        if (data && data->folding.folded) {
            setBlockFolded(block, false);
        }
    }
}

void Qutepart::keyPressEvent(QKeyEvent *event) {
    // Fixme: Handle copy/paste shortcuts
    // We should instead re-connect the original copy/paste actions. Which are
    // not accsible via API.
    if (event->matches(QKeySequence::Copy)) {
        multipleCursorCopy();
        event->accept();
        return;
    } else if (event->matches(QKeySequence::Paste)) {
        multipleCursorPaste();
        event->accept();
        return;
    } else if (event->matches(QKeySequence::Cut)) {
        multipleCursorCut();
        event->accept();
        return;
    }

    QTextCursor cursor = textCursor();

    if (event->key() == Qt::Key_Escape && !extraCursors.isEmpty()) {
        extraCursors.clear();
        updateExtraSelections();
        event->accept();
        return;
    }

    if (event->modifiers() == (Qt::AltModifier | Qt::ShiftModifier)) {
        int offset = 0;
        if (event->key() == Qt::Key_Up) {
            offset = -1;
        } else if (event->key() == Qt::Key_Down) {
            offset = +1;
        }

        if (offset != 0) {
            QTextCursor oldMainCursor = textCursor();
            int currentBlockNumber = oldMainCursor.block().blockNumber();
            int targetBlockNumber = currentBlockNumber + offset;

            if (targetBlockNumber >= 0 && targetBlockNumber < document()->blockCount()) {
                QTextBlock newBlock = document()->findBlockByNumber(targetBlockNumber);
                if (!newBlock.isValid()) {
                    event->ignore();
                    return;
                }
                QTextCursor newMainCursor(newBlock);
                int targetColumn = qMin(oldMainCursor.positionInBlock(), newBlock.length() - 1);
                if (targetColumn < 0) {
                    targetColumn = 0;
                }
                newMainCursor.setPosition(newBlock.position() + targetColumn);

                QSet<int> desiredCursorPositions;

                if (oldMainCursor.position() != newMainCursor.position()) {
                    desiredCursorPositions.insert(oldMainCursor.position());
                }

                for (const auto &ec : extraCursors) {
                    desiredCursorPositions.insert(ec.position());
                }

                desiredCursorPositions.insert(newMainCursor.position());

                QList<QTextCursor> updatedExtraCursors;
                for (int pos : desiredCursorPositions) {
                    if (pos == newMainCursor.position()) {
                        continue;
                    }
                    QTextBlock block = document()->findBlock(pos);
                    if (block.isValid()) {
                        QTextCursor cursor(block);
                        int colInBlock = pos - block.position();
                        if (colInBlock < 0) {
                            colInBlock = 0;
                        }
                        cursor.setPosition(block.position() + qMin(colInBlock, block.length() - 1));
                        updatedExtraCursors.append(cursor);
                    }
                }

                setTextCursor(newMainCursor);
                extraCursors = updatedExtraCursors;
                extraCursorsVisible_ = true;
                viewport()->repaint();
                extraCursorBlinkTimer_->stop();
                extraCursorBlinkTimer_->start();
                updateExtraSelections();
                event->accept();
                return;
            }
        }
    }

    if (!extraCursors.isEmpty()) {
        switch (event->key()) {
        case Qt::Key_Backspace:
        case Qt::Key_Delete: {
            cursor = applyOperationToAllCursors(
                [&](QTextCursor &currentCursor) {
                    if (event->key() == Qt::Key_Backspace) {
                        if (currentCursor.hasSelection()) {
                            currentCursor.deleteChar(); // delete selection
                        } else if (currentCursor.position() > 0) {
                            currentCursor.deletePreviousChar();
                        }
                    } else {
                        currentCursor.deleteChar();
                    }
                },
                [](auto &a, auto &b) { return a.position() > b.position(); });

            setTextCursor(cursor);
            updateExtraSelections();
            update();
            event->accept();
            return;
        }
        case Qt::Key_Left:
        case Qt::Key_Right: {
            cursor = applyOperationToAllCursors(
                [&](QTextCursor &currentCursor) {
                    auto moveMode = QTextCursor::MoveAnchor;
                    if (event->modifiers().testFlag(Qt::ShiftModifier)) {
                        moveMode = QTextCursor::KeepAnchor;
                    }
                    if (event->key() == Qt::Key_Left) {
                        if (event->modifiers().testFlag(Qt::ControlModifier)) {
                            currentCursor.movePosition(QTextCursor::PreviousWord, moveMode);
                        } else {
                            currentCursor.movePosition(QTextCursor::PreviousCharacter, moveMode);
                        }
                    } else { // Qt::Key_Right
                        if (event->modifiers().testFlag(Qt::ControlModifier)) {
                            currentCursor.movePosition(QTextCursor::NextWord, moveMode);
                        } else {
                            currentCursor.movePosition(QTextCursor::NextCharacter, moveMode);
                        }
                    }
                },
                nullptr); // No specific sort order needed for this operation

            setTextCursor(cursor);
            updateExtraSelections();
            event->accept();
            return;
        }
        case Qt::Key_Up:
        case Qt::Key_Down: {
            cursor = applyOperationToAllCursors(
                [&](QTextCursor &currentCursor) {
                    auto moveMode = QTextCursor::MoveAnchor;
                    if (event->modifiers().testFlag(Qt::ShiftModifier)) {
                        moveMode = QTextCursor::KeepAnchor;
                    }
                    auto column = currentCursor.positionInBlock();
                    if (event->key() == Qt::Key_Up) {
                        currentCursor.movePosition(QTextCursor::Up, moveMode);
                    } else {
                        currentCursor.movePosition(QTextCursor::Down, moveMode);
                    }
                    auto newBlockLength = currentCursor.block().length();
                    auto targetColumn = qMin(column, newBlockLength - 1);
                    if (targetColumn < 0) {
                        targetColumn = 0;
                    }
                    currentCursor.setPosition(currentCursor.block().position() + targetColumn,
                                              moveMode);
                },
                [](const auto &a, const auto &b) { return a.position() < b.position(); });

            setTextCursor(cursor);
            updateExtraSelections();
            event->accept();
            return;
        }
        default: {
            // Continue to handle other cases below
        }
        }

        // Handle Enter for multiple cursors
        if (event->matches(QKeySequence::InsertParagraphSeparator)) {
            cursor = applyOperationToAllCursors(
                [&](QTextCursor &currentCursor) {
                    currentCursor.insertBlock();
                    indenter_->indentBlock(currentCursor.block(), currentCursor.positionInBlock(),
                                           QChar::Null);
                },
                [](auto &a, auto &b) { return a.position() > b.position(); });

            setTextCursor(cursor);
            updateExtraSelections();
            update();

            event->accept();
            return;
        }
        // Handle character input
        else if (isCharEvent(event)) {
            auto textToInsert = event->text();
            cursor = applyOperationToAllCursors(
                [&](QTextCursor &currentCursor) { currentCursor.insertText(textToInsert); },
                [](const auto &a, const auto &b) { return a.position() > b.position(); });

            setTextCursor(cursor);

            updateExtraSelections();
            update();

            event->accept();
            return;
        }
    }

    if (softLineWrapping_) {
        if (cursor.columnNumber() >= lineLengthEdge_ && !event->text().isEmpty() &&
            event->key() != Qt::Key_Return && event->key() != Qt::Key_Enter) {
            cursor.select(QTextCursor::WordUnderCursor);
            auto currentWord = cursor.selectedText();
            if (!currentWord.isEmpty()) {
                cursor.beginEditBlock();
                cursor.insertText("\n");
                indenter_->indentBlock(cursor.block(), cursor.positionInBlock(), event->key());
                cursor.insertText(currentWord);
                cursor.insertText(event->text());
                cursor.endEditBlock();
                return;
            }
        }
    }

    if (event->key() == Qt::Key_Backspace && indenter_->shouldUnindentWithBackspace(cursor)) {
        // Unindent on backspace
        indenter_->onShortcutUnindentWithBackspace(cursor);
    } else if (event->matches(QKeySequence::InsertParagraphSeparator)) {
        // Enter pressed. Indent new empty line
        AtomicEditOperation op(this);

        const QString currentLine = cursor.block().text();
        const int cursorPosInBlock = cursor.positionInBlock();

        // Only try to preserve cursor position if it's in the middle of the line
        const bool shouldPreservePosition =
            (cursorPosInBlock > 0) && (cursorPosInBlock < currentLine.length());
        QString textAfterCursor;
        if (shouldPreservePosition) {
            textAfterCursor = currentLine.mid(cursorPosInBlock);
        }

        // Let the default handler process the enter key
        QPlainTextEdit::keyPressEvent(event);

        // Get the new cursor position after the default handler
        QTextCursor newCursor = textCursor();
        const QTextBlock newBlock = newCursor.block();

        // Only apply indentation if we're still on a valid block
        if (newBlock.isValid()) {
            // Apply indentation to the new line
            indenter_->indentBlock(newBlock, newCursor.positionInBlock(), Qt::Key_Enter);

            // If there was text after the cursor, move the cursor back before it
            if (shouldPreservePosition && !textAfterCursor.isEmpty()) {
                const int newPos = newBlock.text().indexOf(textAfterCursor);
                if (newPos != -1) {
                    newCursor.setPosition(newBlock.position() + newPos);
                    setTextCursor(newCursor);
                }
            }
        }
    } else if (cursor.positionInBlock() == (cursor.block().length() - 1) &&
               indenter_->shouldAutoIndentOnEvent(event)) {
        // Indentation on special characters. Like closing bracket in XML
        QPlainTextEdit::keyPressEvent(event);
        indenter_->indentBlock(cursor.block(), cursor.positionInBlock(), event->key());
    } else if (event->key() == Qt::Key_Insert && event->modifiers() == Qt::NoModifier) {
        // toggle Overwrite mode
        setOverwriteMode(!overwriteMode());
    } else if (overwriteMode() && (!event->text().isEmpty()) && isCharEvent(event) &&
               (!cursor.hasSelection()) && cursor.positionInBlock() < cursor.block().length()) {
        // Qt records character replacement in Overwrite mode as 2 overations.
        // but single operation is more preferable.
        AtomicEditOperation op(this);
        QPlainTextEdit::keyPressEvent(event);
    } else if (cursor.hasSelection()) {
        if (event->key() == Qt::Key_Tab && event->modifiers() == Qt::NoModifier) {
            changeSelectedBlocksIndent(true, false);
            event->accept();
            return;
        } else if (event->key() == Qt::Key_Backtab ||
                   (event->key() == Qt::Key_Tab && event->modifiers() == Qt::ShiftModifier)) {
            changeSelectedBlocksIndent(false, false);
            event->accept();
            return;
        }
        switch (event->key()) {
        case Qt::Key_Apostrophe:
            addBrackets(cursor, '\'', '\'');
            setTextCursor(cursor);
            break;
        case Qt::Key_QuoteDbl:
            addBrackets(cursor, '"', '"');
            setTextCursor(cursor);
            break;
        case Qt::Key_ParenLeft:
            addBrackets(cursor, '(', ')');
            setTextCursor(cursor);
            break;
        case Qt::Key_BracketLeft:
            addBrackets(cursor, '[', ']');
            setTextCursor(cursor);
            break;
        case Qt::Key_BraceLeft:
            addBrackets(cursor, '{', '}');
            setTextCursor(cursor);
            break;
        default:
            QPlainTextEdit::keyPressEvent(event);
        }
    } else {
        // make action shortcuts override keyboard events (non-default Qt
        // behaviour)
        for (auto action : actions()) {
            QKeySequence seq = action->shortcut();
            if (seq.count() == 1 && seq[0].key() == event->key() &&
                seq[0].keyboardModifiers() == event->modifiers()) {
                action->trigger();
                return;
            }
        }

        QPlainTextEdit::keyPressEvent(event);
    }
}

void Qutepart::keyReleaseEvent(QKeyEvent *event) {
    if (this->focusWidget() == this) {
        bool textTyped = false;
        if (!event->text().isEmpty()) {
            QChar ch = event->text()[0];
            textTyped = ((event->modifiers() == Qt::NoModifier ||
                          event->modifiers() == Qt::ShiftModifier)) &&
                        (ch.isLetter() || ch.isDigit() || ch == '_');
        }

        if (textTyped || (event->key() == Qt::Key_Backspace && completer_->isVisible())) {
            auto cursor = textCursor();
            cursor.select(QTextCursor::WordUnderCursor);
            auto currentWord = cursor.selectedText();

            if (completionCallback_) {
                auto completions = completionCallback_(currentWord);
                if (!completions.isEmpty()) {
                    completer_->setCustomCompletions(completions);
                }
            }
            completer_->invokeCompletionIfAvailable(false);
        }
    }

    QPlainTextEdit::keyReleaseEvent(event);
}

void Qutepart::paintEvent(QPaintEvent *event) {
    QPlainTextEdit::paintEvent(event);

    // Draw extra cursors if they are visible
    if (!extraCursors.isEmpty() && extraCursorsVisible_) {
        auto painter = QPainter(viewport());
        auto extraCursorColor = Qt::darkCyan;
        painter.setPen(QPen(extraCursorColor, 1));

        for (const auto &extraCursor : extraCursors) {
            auto cursorRect =
                this->cursorRect(extraCursor.block(), extraCursor.positionInBlock(), 0);
            painter.drawLine(cursorRect.topLeft(), cursorRect.bottomLeft());
        }
    }

    drawIndentMarkersAndEdge(event->rect());
}

void Qutepart::changeEvent(QEvent *event) {
    QPlainTextEdit::changeEvent(event);
    if (event->type() == QEvent::FontChange) {
        updateTabStopWidth();

        /* text on line numbers may overlap, if font is bigger, than code font
           Note: the line numbers margin recalculates its width and if it has
               been changed then it calls updateViewport() which in turn will
               update the solid edge line geometry. So there is no need of an
               explicit call self._setSolidEdgeGeometry() here.
         */

        if (bool(lineNumberArea_)) {
            lineNumberArea_->setFont(font());
        }
    }
    updateViewport();
}

void Qutepart::initActions() {
    homeAction_ = createAction("Home", QKeySequence(Qt::Key_Home), QString(),
                               [this]() { this->onShortcutHome(QTextCursor::MoveAnchor); });

    homeSelectAction_ =
        createAction("Home (select)", QKeySequence(Qt::SHIFT | Qt::Key_Home), QString(),
                     [this]() { this->onShortcutHome(QTextCursor::KeepAnchor); });

    endAction_ = createAction("Home", QKeySequence(Qt::Key_End), QString(),
                              [this]() { this->onShortcutEnd(QTextCursor::MoveAnchor); });

    endSelectAction_ =
        createAction("Home (select)", QKeySequence(Qt::SHIFT | Qt::Key_End), QString(),
                     [this]() { this->onShortcutEnd(QTextCursor::KeepAnchor); });

    increaseIndentAction_ =
        createAction("Increase indent", QKeySequence(Qt::Key_Tab), "format-indent-more",
                     [this]() { this->changeSelectedBlocksIndent(true, false); });

    decreaseIndentAction_ =
        createAction("Decrease indent", QKeySequence(Qt::SHIFT | Qt::Key_Tab), "format-indent-less",
                     [this]() { this->changeSelectedBlocksIndent(false, false); });

    toggleBookmarkAction_ = createAction("Toggle bookmark", QKeySequence(Qt::CTRL | Qt::Key_B),
                                         QString(), [this]() { this->onShortcutToggleBookmark(); });

    prevBookmarkAction_ = createAction("Previous bookmark", QKeySequence(Qt::SHIFT | Qt::Key_F2),
                                       QString(), [this]() { this->onShortcutPrevBookmark(); });

    nextBookmarkAction_ = createAction("Next bookmark", QKeySequence(Qt::Key_F2), QString(),
                                       [this]() { this->onShortcutNextBookmark(); });

    invokeCompletionAction_ =
        createAction("Invoke completion", QKeySequence(Qt::CTRL | Qt::Key_Space), QString(),
                     [this]() { this->completer_->invokeCompletion(); });

    duplicateSelectionAction_ =
        createAction("Duplicate selection or line", QKeySequence(Qt::ALT | Qt::Key_D), QString(),
                     [this] { this->duplicateSelection(); });

    moveLineUpAction_ = createAction("Move line up", QKeySequence(Qt::ALT | Qt::Key_Up), QString(),
                                     [this] { this->moveSelectedLines(-1); });
    moveLineDownAction_ = createAction("Move line down", QKeySequence(Qt::ALT | Qt::Key_Down),
                                       QString(), [this] { this->moveSelectedLines(+1); });

    deleteLineAction_ = createAction("Delete line", {}, QString(), [this] { this->deleteLine(); });
    deleteLineAction_->setShortcuts(
        {QKeySequence(Qt::SHIFT | Qt::Key_Delete), QKeySequence(Qt::ALT | Qt::Key_Delete)});

    cutLineAction_ = createAction("Cut line", QKeySequence(Qt::ALT | Qt::Key_X), QString(),
                                  [this] { this->cutLine(); });
    copyLineAction_ = createAction("Copy line", QKeySequence(Qt::ALT | Qt::Key_C), QString(),
                                   [this] { this->copyLine(); });
    pasteLineAction_ = createAction("Paste line", QKeySequence(Qt::ALT | Qt::Key_V), QString(),
                                    [this] { this->pasteLine(); });

    insertLineAboveAction_ =
        createAction("Insert line above", QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_Return),
                     QString(), [this] { this->insertLineAbove(); });
    insertLineBelowAction_ =
        createAction("Insert line below", QKeySequence(Qt::CTRL | Qt::Key_Return), QString(),
                     [this] { this->insertLineBelow(); });

    joinLinesAction_ = createAction("Join lines", QKeySequence(Qt::CTRL | Qt::Key_J), QString(),
                                    [this] { this->onShortcutJoinLines(); });

    scrollDownAction_ = createAction("Scroll down", QKeySequence(Qt::CTRL | Qt::Key_Down),
                                     QString(), [this]() { this->scrollByOffset(1); });
    scrollUpAction_ = createAction("Scroll up", QKeySequence(Qt::CTRL | Qt::Key_Up), QString(),
                                   [this]() { this->scrollByOffset(-1); });

    zoomInAction_ = createAction("Zoom In", QKeySequence(Qt::CTRL | Qt::Key_Equal), QString(),
                                 [this] { this->zoomIn(); });
    zoomOutAction_ = createAction("Zoom Out", QKeySequence(Qt::CTRL | Qt::Key_Minus), QString(),
                                  [this] { this->zoomOut(); });

    toggleActionComment_ =
        createAction(tr("Toggle comment"), QKeySequence(Qt::CTRL | Qt::Key_Slash), {},
                     [this] { this->toggleComment(); });

    findMatchingBracketAction_ = new QAction(tr("Matching bracket"), this);
    findMatchingBracketAction_->setShortcuts({
        QKeySequence(Qt::ALT | Qt::Key_BracketLeft),
        QKeySequence(Qt::ALT | Qt::Key_BracketRight),
        QKeySequence(Qt::ALT | Qt::Key_BraceLeft),
        QKeySequence(Qt::ALT | Qt::Key_BraceRight),
    });
    connect(findMatchingBracketAction_, &QAction::triggered, findMatchingBracketAction_, [this]() {
        if (bracketHighlighter_) {
            auto cursor = textCursor();
            auto position = cursor.positionInBlock();
            auto p1 = TextPosition(cursor.block(), position);
            auto p2 = bracketHighlighter_->getCachedMatch(p1);

            if (!p2.isValid()) {
                p1.column++;
                p2 = bracketHighlighter_->getCachedMatch(p1);
                if (!p2.isValid()) {
                    p1.column -= 2;
                    p2 = bracketHighlighter_->getCachedMatch(p1);
                }
            }
            if (p2.isValid()) {
                auto shiftPressed = QApplication::keyboardModifiers().testFlag(Qt::ShiftModifier);

                if (shiftPressed) {
                    cursor.setPosition(p1.block.position() + p1.column);
                    cursor.setPosition(p2.block.position() + p2.column + 1,
                                       QTextCursor::KeepAnchor);
                    setTextCursor(cursor);
                } else {
                    goTo(p2.block.blockNumber(), p2.column);
                }
            }
        }
    });

    foldAction_ = new QAction(tr("Fold current block"), this);
    foldAction_->setShortcutContext(Qt::WidgetShortcut);
    foldAction_->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_BracketLeft));
    connect(foldAction_, &QAction::triggered, this, &Qutepart::foldCurrentBlock);
    this->addAction(foldAction_);

    unfoldAction_ = new QAction(tr("Unfold current block"), this);
    unfoldAction_->setShortcutContext(Qt::WidgetShortcut);
    unfoldAction_->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_BracketRight));
    connect(unfoldAction_, &QAction::triggered, this, &Qutepart::unfoldCurrentBlock);
    this->addAction(unfoldAction_);

    toggleFoldAction_ = new QAction(tr("Toggle fold current block"), this);
    toggleFoldAction_->setShortcutContext(Qt::WidgetShortcut);
    toggleFoldAction_->setShortcut(QKeySequence(Qt::ALT | Qt::Key_Asterisk));
    connect(toggleFoldAction_, &QAction::triggered, this, &Qutepart::toggleCurrentFold);
    this->addAction(toggleFoldAction_);

    foldTopLevelAction_ = new QAction(tr("Fold top level blocks"), this);
    foldTopLevelAction_->setShortcutContext(Qt::WidgetShortcut);
    foldTopLevelAction_->setShortcut(QKeySequence(Qt::ALT | Qt::Key_0));
    connect(foldTopLevelAction_, &QAction::triggered, this, &Qutepart::foldTopLevelBlocks);
    this->addAction(foldTopLevelAction_);

    unfoldAllAction_ = new QAction(tr("Unfold all"), this);
    unfoldAllAction_->setShortcutContext(Qt::WidgetShortcut);
    unfoldAllAction_->setShortcut(QKeySequence(Qt::ALT | Qt::SHIFT | Qt::Key_0));
    connect(unfoldAllAction_, &QAction::triggered, this, &Qutepart::unfoldAll);
    this->addAction(unfoldAllAction_);
}

QAction *Qutepart::createAction(const QString &text, QKeySequence shortcut,
                                const QString & /*iconFileName*/,
                                std::function<void()> const &handler) {

    QAction *action = new QAction(text, this);

#if 0 // TODO
    if iconFileName is not None:
         action.setIcon(getIcon(iconFileName))
#endif
    action->setShortcut(shortcut);
    action->setShortcutContext(Qt::WidgetShortcut);
    connect(action, &QAction::triggered, handler);
    addAction(action);
    return action;
}

void Qutepart::drawIndentMarkersAndEdge(const QRect &paintEventRect) {
    QPainter painter(viewport());

    if (drawSolidEdge_) {
        painter.setPen(lineLengthEdgeColor_);
        QRect cr = contentsRect();
        int x = fontMetrics().horizontalAdvance(QString().fill('9', lineLengthEdge_)) +
                cursorRect(firstVisibleBlock(), 0, 0).left();
        painter.drawLine(QPoint(x + 1, cr.top()), QPoint(x + 1, cr.bottom()));
    }

    for (QTextBlock block = firstVisibleBlock(); block.isValid(); block = block.next()) {
        QRectF blockGeometry = blockBoundingGeometry(block).translated(contentOffset());
        if (blockGeometry.top() > paintEventRect.bottom()) {
            break;
        }

        if (block.isVisible() && blockGeometry.toRect().intersects(paintEventRect)) {
            // Draw indent markers, if good indentation is not drawn
            if (drawIndentations_ && (!drawAnyWhitespace_)) {
                QString text = block.text();
                QStringView textRef(text);
                int column = indenter_->width();
                while (textRef.startsWith(indenter_->indentText()) &&
                       textRef.length() > indenter_->width() &&
                       textRef.at(indenter_->width()).isSpace()) {
                    bool lineLengthMarkerHere = (column == lineLengthEdge_);
                    bool cursorHere = (block.blockNumber() == textCursor().blockNumber() &&
                                       column == textCursor().columnNumber());
                    if ((!lineLengthMarkerHere) && (!cursorHere)) { // looks ugly, if both drawn
                        // on some fonts line is drawn below the cursor, if
                        // offset is 1. Looks like Qt bug
                        drawIndentMarker(&painter, block, column);
                    }

                    textRef = textRef.mid(indenter_->width());
                    column += indenter_->width();
                }
            }

            if (drawAnyWhitespace_ || drawIncorrectIndentation_) {
                QString text = block.text();
                QVector<bool> visibleFlags(text.length());
                chooseVisibleWhitespace(text, &visibleFlags);
                for (int column = 0; column < visibleFlags.length(); column++) {
                    bool draw = visibleFlags[column];
                    if (draw) {
                        drawWhiteSpace(&painter, block, column, text[column]);
                    }
                }
            }
        }
    }
}

void Qutepart::drawWhiteSpace(QPainter *painter, QTextBlock block, int column, QChar ch) {
    if (!block.isValid()) {
        // Should not happen, but adding a check to prevent crash
        qDebug() << "Invalid block in drawWhiteSpace!";
        return;
    }
    auto leftCursorRect = cursorRect(block, column, 0);
    auto rightCursorRect = cursorRect(block, column + 1, 0);
    if (leftCursorRect.top() == rightCursorRect.top()) {
        auto middleHeight = (leftCursorRect.top() + leftCursorRect.bottom()) / 2;
        auto oldMode = painter->compositionMode();
        painter->setCompositionMode(QPainter::CompositionMode_SourceOver);
        painter->setPen(whitespaceColor_);
        if (ch == ' ') {
            auto xPos = (leftCursorRect.x() + rightCursorRect.x()) / 2;
            painter->drawRect(QRect(xPos, middleHeight, 1, 1));
        } else {
            painter->drawLine(leftCursorRect.x() + 3, middleHeight, rightCursorRect.x() - 3,
                              middleHeight);
        }
        painter->setCompositionMode(oldMode);
    }
}

int Qutepart::effectiveEdgePos(const QString &text) {
    /* Position of edge in a block.
     * Defined by lineLengthEdge, but visible width of \t is more than 1,
     * therefore effective position depends on count and position of \t symbols
     * Return -1 if line is too short to have edge
     */
    if (lineLengthEdge_ <= 0) {
        return -1;
    }

    int tabExtraWidth = indenter_->width() - 1;
    int fullWidth = text.length() + (text.count('\t') * tabExtraWidth);
    int indentWidth = indenter_->width();

    if (fullWidth <= lineLengthEdge_) {
        return -1;
    }

    int currentWidth = 0;
    for (int pos = 0; pos < text.length(); pos++) {
        if (text[pos] == '\t') {
            // Qt indents up to indentation level, so visible \t width depends
            // on position
            currentWidth += (indentWidth - (currentWidth % indentWidth));
        } else {
            currentWidth += 1;
        }
        if (currentWidth > lineLengthEdge_) {
            return pos;
        }
    }

    // line too narrow, probably visible \t width is small
    return -1;
}

/**
 * @brief Determines which whitespace characters should be visible in the given text.
 *
 * This function analyzes the input text and marks whitespace characters that should
 * be visually represented based on the current settings.
 *
 * @param text The input text to analyze.
 * @param result Pointer to a QVector<bool> that will be filled with the visibility
 *               status of each character. True means the whitespace should be visible.
 *
 * @details The function behaves as follows:
 * - Whitespace at the start and end of the line is always marked as visible.
 * - In the middle of the line, only consecutive whitespace (2 or more spaces) is marked.
 * - If drawAnyWhitespace_ is true, all whitespace characters are marked as per the above rules.
 * - If drawIncorrectIndentation_ is true, incorrect indentation is additionally marked:
 *   - For tab-based indentation: groups of spaces as wide as a tab are marked.
 *   - For space-based indentation: all tabs are marked.
 *
 * The function only processes whitespace if either drawAnyWhitespace_ or
 * drawIncorrectIndentation_ is true. If both are false, no whitespace will be marked.
 *
 * @note The function resizes the result vector to match the length of the input text.
 * @note The function does nothing if the input text is empty.
 *
 * @see drawAnyWhitespace_
 * @see drawIncorrectIndentation_
 * @see indenter_
 */
void Qutepart::chooseVisibleWhitespace(const QString &text, QVector<bool> *result) {
    if (text.isEmpty()) {
        return;
    }

    result->resize(text.length());
    result->fill(false);

    int lastNonWhitespaceIndex = text.length() - 1;
    while (lastNonWhitespaceIndex >= 0 && text[lastNonWhitespaceIndex].isSpace()) {
        lastNonWhitespaceIndex--;
    }

    if (drawAnyWhitespace_ || drawIncorrectIndentation_) {
        // Mark start whitespace
        int startWhitespace = 0;
        while (startWhitespace < text.length() && text[startWhitespace].isSpace()) {
            result->replace(startWhitespace, true);
            startWhitespace++;
        }

        // Mark end whitespace
        for (int i = lastNonWhitespaceIndex + 1; i < text.length(); i++) {
            result->replace(i, true);
        }

        // Mark middle whitespace
        for (int i = startWhitespace; i <= lastNonWhitespaceIndex; i++) {
            if (text[i].isSpace()) {
                if (i + 1 < text.length() && text[i + 1].isSpace()) {
                    // Mark consecutive whitespace
                    int j = i;
                    while (j < text.length() && text[j].isSpace()) {
                        result->replace(j, true);
                        j++;
                    }
                    i = j - 1;
                }
            }
        }

        if (drawIncorrectIndentation_) {
            // Mark incorrect indentation
            if (indenter_->useTabs()) {
                // Find big space groups
                QString bigSpaceGroup = QString(indenter_->width(), ' ');
                for (int column = text.indexOf(bigSpaceGroup);
                     column != -1 && column <= lastNonWhitespaceIndex;
                     column = text.indexOf(bigSpaceGroup, column + 1)) {
                    for (int index = column; index < column + indenter_->width(); index++) {
                        result->replace(index, true);
                    }
                }
            } else {
                // Find tabs
                for (int column = text.indexOf('\t');
                     column != -1 && column <= lastNonWhitespaceIndex;
                     column = text.indexOf('\t', column + 1)) {
                    result->replace(column, true);
                }
            }
        }
    }
}

QTextEdit::ExtraSelection Qutepart::currentLineExtraSelection() const {
    QTextEdit::ExtraSelection selection;

    selection.format.setBackground(currentLineColor_);
    selection.format.setProperty(QTextFormat::FullWidthSelection, true);
    selection.cursor = textCursor();
    selection.cursor.clearSelection();

    return selection;
}

void Qutepart::updateViewport() {
    auto cr = contentsRect();
    auto currentX = cr.left();
    auto top = cr.top();
    auto height = cr.height();
    auto viewportMarginStart = 0;
    auto viewportMarginEnd = 0;
    auto deltaOrizontal = verticalScrollBar()->isVisible() ? verticalScrollBar()->width() : 0;

    if (lineNumberArea_) {
        auto width = lineNumberArea_->widthHint();
        lineNumberArea_->setGeometry(QRect(currentX, top, width, height));
        currentX += width;
        viewportMarginStart += width;
    }

    {
        auto width = markArea_->widthHint();
        markArea_->setGeometry(QRect(currentX, top, width, height));
        viewportMarginStart += width;
        currentX += width;
    }

    if (foldingArea_) {
        auto width = foldingArea_->widthHint();
        foldingArea_->setGeometry(QRect(currentX, top, width, height));
        viewportMarginStart += width;
    }

    if (miniMap_) {
        auto mainWidth = cr.width();
        auto width = miniMap_->widthHint();
        auto shouldHide = mainWidth < width * 4;

        if (shouldHide) {
            miniMap_->hide();
        } else {
            miniMap_->show();
            miniMap_->setGeometry(QRect(cr.width() - width - deltaOrizontal, top, width, height));
            viewportMarginEnd += width;
        }
    }

    if (viewportMarginStart_ != viewportMarginStart || viewportMarginEnd_ != viewportMarginEnd) {
        viewportMarginStart_ = viewportMarginStart;
        viewportMarginEnd_ = viewportMarginEnd;
        setViewportMargins(viewportMarginStart_, 0, viewportMarginEnd_, 0);
    }
}

void Qutepart::resizeEvent(QResizeEvent *event) {
    QPlainTextEdit::resizeEvent(event);
    updateViewport();
}

void Qutepart::updateTabStopWidth() {
    auto width = fontMetrics().horizontalAdvance(QString().fill(' ', indenter_->width()));
    setTabStopDistance(width);
}

void Qutepart::drawIndentMarker(QPainter *painter, QTextBlock block, int column) {
    painter->setPen(indentColor_);
    QRect rect = cursorRect(block, column, 0);
    painter->drawLine(rect.topLeft(), rect.bottomLeft());
}

void Qutepart::drawEdgeLine(QPainter *painter, QTextBlock block, int edgePos) {
    painter->setPen(QPen(QBrush(lineLengthEdgeColor_), 0));
    QRect rect = cursorRect(block, edgePos, 0);
    painter->drawLine(rect.topLeft(), rect.bottomLeft());
}

QRect Qutepart::cursorRect(QTextBlock block, int column, int offset) const {
    QTextCursor cursor(block);
    setPositionInBlock(&cursor, column);
    return QPlainTextEdit::cursorRect(cursor).translated(offset, 0);
}

void Qutepart::gotoBlock(const QTextBlock &block) {
    QTextCursor cursor(block);
    setTextCursor(cursor);
}

namespace {
QTextCursor cursorAtSpaceEnd(const QTextBlock &block) {
    QTextCursor cursor(block);
    setPositionInBlock(&cursor, blockIndent(block).length());
    return cursor;
}

} // anonymous namespace

void Qutepart::indentBlock(const QTextBlock &block, bool withSpace) const {
    QTextCursor cursor(cursorAtSpaceEnd(block));
    if (withSpace) {
        cursor.insertText(" ");
    } else {
        cursor.insertText(indenter_->indentText());
    }
}

void Qutepart::unIndentBlock(const QTextBlock &block, bool withSpace) const {
    QString currentIndent = blockIndent(block);

    int charsToRemove = -1;

    if (currentIndent.endsWith('\t')) {
        charsToRemove = 1;
    } else if (withSpace) {
        charsToRemove = std::min(qsizetype(1), currentIndent.length());
    } else {
        if (indenter_->useTabs()) {
            charsToRemove = std::min(spaceAtEndCount(currentIndent), indenter_->width());
        } else {                                                   // spaces
            if (currentIndent.endsWith(indenter_->indentText())) { // remove indent level
                charsToRemove = indenter_->width();
            } else { // remove all spaces
                charsToRemove = std::min(spaceAtEndCount(currentIndent), indenter_->width());
            }
        }
    }

    if (charsToRemove > 0) {
        QTextCursor cursor(cursorAtSpaceEnd(block));
        cursor.setPosition(cursor.position() - charsToRemove, QTextCursor::KeepAnchor);
        cursor.removeSelectedText();
    }
}

/* Tab or Space pressed and few blocks are selected, or Shift+Tab pressed
Insert or remove text from the beginning of blocks
*/
void Qutepart::changeSelectedBlocksIndent(bool increase, bool withSpace) {
    QTextCursor cursor = textCursor();

    // Handle both single and multiple cursors
    cursor = applyOperationToAllCursors(
        [&](QTextCursor &currentCursor) {
            if (currentCursor.hasSelection()) {
                QTextBlock startBlock = document()->findBlock(currentCursor.selectionStart());
                QTextBlock endBlock = document()->findBlock(currentCursor.selectionEnd());
                if (currentCursor.selectionStart() != currentCursor.selectionEnd() &&
                    endBlock.position() == currentCursor.selectionEnd() &&
                    endBlock.previous().isValid()) {
                    // do not indent not selected line if indenting multiple lines
                    endBlock = endBlock.previous();
                }

                if (startBlock == endBlock) {
                    // indent single line
                    if (increase) {
                        indentBlock(startBlock, withSpace);
                    } else {
                        unIndentBlock(startBlock, withSpace);
                    }
                } else {
                    // indent multiply lines
                    QTextBlock stopBlock = endBlock.next();
                    QTextBlock block = startBlock;
                    while (block != stopBlock) {
                        if (increase) {
                            indentBlock(block, withSpace);
                        } else {
                            unIndentBlock(block, withSpace);
                        }
                        block = block.next();
                    }
                }
            } else {
                // No selection
                if (increase) {
                    indenter_->onShortcutIndentAfterCursor(currentCursor);
                } else {
                    indenter_->onShortcutUnindentWithBackspace(currentCursor);
                }
            }
        },
        [](auto &a, auto &b) { return a.position() > b.position(); });

    setTextCursor(cursor);
    updateExtraSelections();
    update();
}

void Qutepart::scrollByOffset(int offset) {
    int value = verticalScrollBar()->value();
    value += offset;
    verticalScrollBar()->setValue(value);
}

void Qutepart::duplicateSelection() {
    AtomicEditOperation op(this);
    auto cursor = textCursor();

    if (cursor.hasSelection()) {
        // duplicate selection
        auto text = cursor.selectedText();
        cursor.setPosition(std::max(cursor.position(), cursor.anchor()));
        auto anchor = cursor.position();
        cursor.insertText(text);
        auto pos = cursor.position();
        cursor.setPosition(anchor);
        cursor.setPosition(pos, QTextCursor::KeepAnchor);
        setTextCursor(cursor);
    } else {
        // duplicate current line
        auto cursorColumn = cursor.positionInBlock();
        auto text = cursor.block().text();
        cursor.movePosition(QTextCursor::EndOfBlock);
        cursor.insertBlock();
        cursor.insertText(text);
        cursor.movePosition(QTextCursor::StartOfBlock);
        cursor.setPosition(cursor.position() + cursorColumn);
        setTextCursor(cursor);
    }
}

void Qutepart::moveSelectedLines(int offsetLines) {
    if (offsetLines == 0) {
        return;
    }

    auto minSelectedBlock = INT_MAX;
    auto maxSelectedBlock = INT_MIN;
    auto allCursors = extraCursors;
    auto originalSelections = QVector<QPair<int, int>>();

    allCursors.append(textCursor());
    for (const auto &cursor : allCursors) {
        auto posBlock = cursor.block().blockNumber();
        auto anchorBlock = document()->findBlock(cursor.anchor()).blockNumber();
        auto startBlock = std::min(posBlock, anchorBlock);
        auto endBlock = std::max(posBlock, anchorBlock);
        minSelectedBlock = std::min(minSelectedBlock, startBlock);
        maxSelectedBlock = std::max(maxSelectedBlock, endBlock);
        originalSelections.append({startBlock, endBlock});
    }

    auto line = (offsetLines < 0) ? minSelectedBlock - 1 : maxSelectedBlock + 1;
    if (line >= 0 && line < document()->blockCount()) {
        auto block = document()->findBlockByNumber(line);
        if (block.isValid() && !block.isVisible()) {
            while (block.isValid() && !block.isVisible()) {
                block = (offsetLines < 0) ? block.previous() : block.next();
                if (block.isValid()) {
                    unfoldBlock(block.blockNumber());
                }
            }
        }
    }

    if (offsetLines < 0 && minSelectedBlock == 0) {
        return;
    }
    if (offsetLines > 0 && maxSelectedBlock == document()->blockCount() - 1) {
        return;
    }
    QVector<QString> linesContentSnapshot;
    for (auto i = 0; i < lines().count(); ++i) {
        linesContentSnapshot.append(lines().at(i).text());
    }

    QMap<int, int> originalBlockToFinalPhysicalIndexMap;
    auto cursor = applyOperationToAllCursors(
        [&](QTextCursor &cursor) {
            auto posBlock = cursor.block().blockNumber();
            auto anchorBlock = document()->findBlock(cursor.anchor()).blockNumber();
            auto startBlock = std::min(posBlock, anchorBlock);
            auto endBlock = std::max(posBlock, anchorBlock);
            if (offsetLines < 0 && startBlock == 0) {
                return cursor;
            }
            if (offsetLines > 0 && endBlock == document()->blockCount() - 1) {
                return cursor;
            }
            auto column = cursor.positionInBlock();
            auto targetStartBlock = startBlock + offsetLines;
            for (int i = startBlock; i <= endBlock; ++i) {
                originalBlockToFinalPhysicalIndexMap[i] = targetStartBlock + (i - startBlock);
            }
            auto linesToMove = QStringList();
            for (auto i = startBlock; i <= endBlock; ++i) {
                linesToMove.append(linesContentSnapshot[i]);
            }
            for (auto i = endBlock; i >= startBlock; --i) {
                lines().popAt(i);
            }
            for (auto i = 0; i < linesToMove.size(); ++i) {
                lines().insertAt(targetStartBlock + i, linesToMove[i]);
            }
            auto targetBlock = document()->findBlockByNumber(targetStartBlock);
            cursor.setPosition(targetBlock.position() + qMin(column, targetBlock.length() - 1));
            return cursor;
        },
        [offsetLines](const QTextCursor &a, const QTextCursor &b) {
            // When moving down, process from bottom to top
            // When moving up, process from top to bottom
            return offsetLines > 0 ? a.block().blockNumber() > b.block().blockNumber()
                                   : a.block().blockNumber() < b.block().blockNumber();
        });

    // Reapply selections after move
    auto newCursors = QList<QTextCursor>();
    for (const auto &[startBlock, endBlock] : originalSelections) {
        auto cursor = QTextCursor(document());
        auto newStartBlock = startBlock + offsetLines;
        auto newEndBlock = endBlock + offsetLines;
        auto startPos = document()->findBlockByNumber(newStartBlock).position();
        auto endBlockObj = document()->findBlockByNumber(newEndBlock);
        auto endPos = endBlockObj.position() + endBlockObj.length() - 1;
        cursor.setPosition(startPos);
        cursor.setPosition(endPos, QTextCursor::KeepAnchor);
        newCursors.append(cursor);
    }

    if (!newCursors.isEmpty()) {
        setTextCursor(newCursors.last());
        extraCursors = newCursors.mid(0, newCursors.size() - 1);
    }

    updateExtraSelections();
    markArea_->update();
    ensureCursorVisible();
}

void Qutepart::deleteLine() {
    QTextCursor cursor = textCursor();
    int posBlock = cursor.block().blockNumber();
    int anchorBlock = document()->findBlock(cursor.anchor()).blockNumber();
    int startBlock = std::min(posBlock, anchorBlock);
    int endBlock = std::max(posBlock, anchorBlock);

    AtomicEditOperation op(this);
    for (int i = endBlock; i >= startBlock; i--) {
        lines().popAt(i);
    }

    if (anchorBlock != 0) {
        cursor.movePosition(QTextCursor::NextBlock);
    }

    setTextCursor(cursor);
}

void Qutepart::cutLine() {
    copyLine();
    deleteLine();
}

void Qutepart::copyLine() {
    QTextCursor cursor = textCursor();

    int smallerPos = std::min(cursor.anchor(), cursor.position());
    int biggerPos = std::max(cursor.anchor(), cursor.position());

    QTextBlock block = document()->findBlock(smallerPos);
    QTextBlock lastBlock = document()->findBlock(biggerPos);

    QStringList lines;
    while (block.isValid() && block.blockNumber() <= lastBlock.blockNumber()) {
        QString text = block.text();
        if (text.endsWith("\u2029")) {
            text = text.left(text.length() - 1);
        }
        lines << text;
        block = block.next();
    }

    QString textToCopy = lines.join('\n');

    QApplication::clipboard()->setText(textToCopy);
}

void Qutepart::pasteLine() {
    QTextCursor cursor = textCursor();
    cursor.movePosition(QTextCursor::EndOfBlock);

    AtomicEditOperation op(this);
    cursor.insertBlock();
    cursor.insertText(QApplication::clipboard()->text());
}

void Qutepart::insertLineAbove() {
    QTextCursor cursor = textCursor();

    AtomicEditOperation op(this);

    if (cursor.blockNumber() == 0) {
        cursor.movePosition(QTextCursor::StartOfBlock);
        cursor.insertBlock();
        cursor.movePosition(QTextCursor::PreviousBlock);
    } else {
        cursor.movePosition(QTextCursor::StartOfBlock);
        cursor.movePosition(QTextCursor::Left);
        cursor.insertBlock();
    }

    setTextCursor(cursor);
    autoIndentCurrentLine();
}

void Qutepart::insertLineBelow() {
    QTextCursor cursor = textCursor();

    AtomicEditOperation op(this);

    cursor.movePosition(QTextCursor::EndOfBlock);
    cursor.insertBlock();

    setTextCursor(cursor);
    autoIndentCurrentLine();
}

auto static splitLeadingWhitespace(const QString &str) -> std::tuple<QString, QString> {
    int i = 0;
    while (i < str.length() && str[i].isSpace()) {
        ++i;
    }
    return {str.left(i), str.mid(i)};
}

void Qutepart::toggleComment() {
    auto hl = dynamic_cast<SyntaxHighlighter *>(highlighter_);
    if (!hl) {
        return;
    }

    AtomicEditOperation op(this);
    auto cursor = textCursor();
    auto selectionStart = cursor.selectionStart();
    auto selectionEnd = cursor.selectionEnd();

    auto data = static_cast<TextBlockUserData *>(cursor.block().userData());
    if (!data || !data->contexts.currentContext() || !data->contexts.currentContext()->language) {
        return;
    }
    auto language = data->contexts.currentContext()->language;

    cursor.setPosition(selectionStart);
    auto startComment = language->getStartMultilineComment();
    auto endComment = language->getEndMultilineComment();
    auto singleLineComment = language->getSingleLineComment();

    auto handleSingleLineComment = [&]() {
        auto originalPosition = cursor.position();
        cursor.movePosition(QTextCursor::StartOfLine);
        cursor.movePosition(QTextCursor::EndOfLine, QTextCursor::KeepAnchor);
        auto [indentation, text] = splitLeadingWhitespace(cursor.selectedText());

#if 0
        if (text.startsWith(singleLineComment)) {
            text = text.mid(singleLineComment.length());
            originalPosition -= singleLineComment.length();
        } else {
            text = singleLineComment + text;
            originalPosition += singleLineComment.length();
        }
#endif
        if (!singleLineComment.isEmpty()) {
            if (text.startsWith(singleLineComment)) {
                text = text.mid(singleLineComment.length());
                originalPosition -= singleLineComment.length();
            } else {
                text = singleLineComment + text;
                originalPosition += singleLineComment.length();
            }
        } else if (!startComment.isEmpty() && !endComment.isEmpty()) {
            if (text.startsWith(startComment) && text.endsWith(endComment)) {
                text = text.mid(startComment.length(),
                                text.length() - startComment.length() - endComment.length());
                originalPosition -= startComment.length();
            } else {
                text = startComment + text + endComment;
                originalPosition += startComment.length();
            }
        }
        cursor.removeSelectedText();
        cursor.insertText(indentation + text);
        cursor.setPosition(originalPosition);
    };

    auto handleMultilineComment = [&]() {
        cursor.setPosition(selectionStart);
        cursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor, startComment.length());
        auto startsWithComment = (cursor.selectedText().startsWith(startComment));

        cursor.setPosition(selectionEnd - endComment.length());
        cursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor, endComment.length());
        auto endsWithComment = (cursor.selectedText().endsWith(endComment));

        if (startsWithComment && endsWithComment) {
            cursor.setPosition(selectionStart);
            cursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor, startComment.length());
            cursor.removeSelectedText();
            cursor.setPosition(selectionEnd - startComment.length() - endComment.length());
            cursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor, endComment.length());
            cursor.removeSelectedText();
            selectionEnd -= (startComment.length() + endComment.length());
        } else {
            cursor.setPosition(selectionEnd);
            cursor.insertText(endComment);
            cursor.setPosition(selectionStart);
            cursor.insertText(startComment);
            selectionEnd += (startComment.length() + endComment.length());
        }

        cursor.setPosition(selectionStart);
        cursor.setPosition(selectionEnd, QTextCursor::KeepAnchor);
    };

    auto handleMultilineCommentSingleLines = [&]() {
        auto startBlock = cursor.document()->findBlock(selectionStart).blockNumber();
        auto endBlock = cursor.document()->findBlock(selectionEnd).blockNumber();
        auto allNonEmptyLinesCommented = true;

        cursor.setPosition(selectionStart);
        for (auto i = startBlock; i <= endBlock; ++i) {
            auto line = cursor.block().text().trimmed();
            if (!line.isEmpty() && !line.startsWith(singleLineComment)) {
                allNonEmptyLinesCommented = false;
                break;
            }
            cursor.movePosition(QTextCursor::NextBlock);
        }

        cursor.setPosition(selectionStart);
        for (auto i = startBlock; i <= endBlock; ++i) {
            cursor.movePosition(QTextCursor::StartOfLine);
            auto line = cursor.block().text();
            auto trimmedLine = line.trimmed();

            if (allNonEmptyLinesCommented) {
                if (trimmedLine.startsWith(singleLineComment)) {
                    int index = line.indexOf(singleLineComment);
                    cursor.movePosition(QTextCursor::Right, QTextCursor::MoveAnchor, index);
                    cursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor,
                                        singleLineComment.length());
                    cursor.removeSelectedText();
                }
            } else {
                if (!trimmedLine.isEmpty() && !trimmedLine.startsWith(singleLineComment)) {
                    cursor.insertText(singleLineComment);
                }
            }

            if (i < endBlock) {
                cursor.movePosition(QTextCursor::NextBlock);
            }
        }

        cursor.setPosition(selectionStart);
        cursor.setPosition(selectionEnd, QTextCursor::KeepAnchor);
        auto newSelectedText = cursor.selectedText();
        auto lineSeparatorCount = newSelectedText.count(QChar::ParagraphSeparator);
        auto newSelectionEnd = cursor.position();

        if (allNonEmptyLinesCommented) {
            newSelectionEnd -= singleLineComment.length() * (lineSeparatorCount + 1);
        } else {
            newSelectionEnd += singleLineComment.length() * (lineSeparatorCount + 1);
        }
        cursor.setPosition(selectionStart);
        cursor.setPosition(newSelectionEnd, QTextCursor::KeepAnchor);
    };

    if (selectionStart != selectionEnd) {
        if (startComment.isEmpty() && endComment.isEmpty()) {
            handleMultilineCommentSingleLines();
        } else {
            handleMultilineComment();
        }
    } else {
        handleSingleLineComment();
    }

    setTextCursor(cursor);
}

void Qutepart::updateExtraSelections() {
    auto cursor = textCursor();
    auto selections = persitentSelections;

    if (bracketHighlighter_) {
        selections.append(bracketHighlighter_->extraSelections(
            TextPosition(textCursor().block(), cursor.positionInBlock())));
    }

    if (extraCursors.isEmpty()) {
        // Highlight current line when there's only a single cursor.
        if (currentLineColor_.isValid()) {
            selections.append(currentLineExtraSelection());
        }

        if (getMarkCurrentWord()) {
            if (cursor.hasSelection()) {
                auto selectedText = cursor.selectedText();
                if (!selectedText.isEmpty()) {
                    auto searches = highlightText(selectedText, false);
                    selections += searches;
                }
            } else if (lastWordUnderCursor.length() > 2) {
                auto searches = highlightText(lastWordUnderCursor, true);
                selections += searches;
            }
        }
    } else {
        for (const auto &extraCursor : extraCursors) {
            if (extraCursor.hasSelection()) {
                auto extraSelection = QTextEdit::ExtraSelection();
                extraSelection.format.setBackground(
                    QApplication::palette().color(QPalette::Highlight));
                extraSelection.format.setProperty(QTextFormat::FullWidthSelection, false);
                extraSelection.cursor = extraCursor;
                selections.append(extraSelection);
            }
        }
    }

    setExtraSelections(selections);
}

/// Smart Home behaviour. Move to first non-space or to beginning of line
void Qutepart::onShortcutHome(QTextCursor::MoveMode moveMode) {
    auto mainCursor = textCursor();
    auto firstNonSpace = firstNonSpaceColumn(mainCursor.block().text());
    if (enableSmartHomeEnd_ && mainCursor.positionInBlock() == firstNonSpace) {
        setPositionInBlock(&mainCursor, 0, moveMode);
    } else {
        setPositionInBlock(&mainCursor, firstNonSpace, moveMode);
    }
    setTextCursor(mainCursor);

    for (auto &extraCursor : extraCursors) {
        firstNonSpace = firstNonSpaceColumn(extraCursor.block().text());
        int targetPosition;
        if (enableSmartHomeEnd_ && extraCursor.positionInBlock() == firstNonSpace) {
            targetPosition = extraCursor.block().position() + 0;
        } else {
            targetPosition = extraCursor.block().position() + firstNonSpace;
        }
        extraCursor.setPosition(targetPosition, moveMode);
    }
    updateExtraSelections();
}

/// Smart end behaviour. Move to last non-space or to end of line
void Qutepart::onShortcutEnd(QTextCursor::MoveMode moveMode) {
    auto mainCursor = textCursor();
    auto lastNonSpace = lastNonSpaceColumn(mainCursor.block().text()) + 1;
    auto lastChar = mainCursor.block().length() - 1;
    if (lastNonSpace > lastChar) {
        lastNonSpace = lastChar;
    }
    if (enableSmartHomeEnd_ && mainCursor.positionInBlock() == lastNonSpace) {
        setPositionInBlock(&mainCursor, lastChar, moveMode);
    } else {
        setPositionInBlock(&mainCursor, lastNonSpace, moveMode);
    }
    setTextCursor(mainCursor);

    for (auto &extraCursor : extraCursors) {
        lastNonSpace = lastNonSpaceColumn(extraCursor.block().text()) + 1;
        lastChar = extraCursor.block().length() - 1;
        if (lastNonSpace > lastChar) {
            lastNonSpace = lastChar;
        }
        int targetPosition;
        if (enableSmartHomeEnd_ && extraCursor.positionInBlock() == lastNonSpace) {
            targetPosition = extraCursor.block().position() + lastChar;
        } else {
            targetPosition = extraCursor.block().position() + lastNonSpace;
        }
        extraCursor.setPosition(targetPosition, moveMode);
    }
    updateExtraSelections();
}

void Qutepart::onShortcutToggleBookmark() {
    auto block = textCursor().block();
    auto value = hasFlag(block, BOOMARK_BIT);
    setFlag(block, BOOMARK_BIT, !value);
    markArea_->update();
}

void Qutepart::onShortcutPrevBookmark() {
    QTextBlock currentBlock = textCursor().block();
    QTextBlock block = currentBlock.previous();

    while (block.isValid()) {
        if (isBookmarked(block)) {
            gotoBlock(block);
            return;
        }
        block = block.previous();
    }
    block = document()->lastBlock();
    while (block != currentBlock) {
        if (isBookmarked(block)) {
            gotoBlock(block);
            return;
        }
        block = block.previous();
    }
}

void Qutepart::onShortcutNextBookmark() {
    QTextBlock currentBlock = textCursor().block();
    QTextBlock block = currentBlock.next();

    while (block.isValid()) {
        if (isBookmarked(block)) {
            gotoBlock(block);
            return;
        }
        block = block.next();
    }

    block = document()->firstBlock();
    while (block != currentBlock) {
        if (isBookmarked(block)) {
            gotoBlock(block);
            return;
        }
        block = block.next();
    }
}

// Helper function for onShortcutJoinLines()
void Qutepart::joinNextLine(QTextCursor &cursor) {
    cursor.movePosition(QTextCursor::EndOfBlock);
    cursor.movePosition(QTextCursor::NextBlock, QTextCursor::KeepAnchor);

    setPositionInBlock(&cursor, firstNonSpaceColumn(cursor.block().text()),
                       QTextCursor::KeepAnchor);
    cursor.insertText(" ");
}

void Qutepart::onShortcutJoinLines() {
    QTextCursor cursor = textCursor();
    if (cursor.hasSelection()) {
        QTextCursor editCursor(document());
        editCursor.setPosition(std::min(cursor.position(), cursor.anchor()));
        int posBlockNumber = cursor.blockNumber();
        int anchorBlockNumber = document()->findBlock(cursor.anchor()).blockNumber();
        int joinCount = std::abs(posBlockNumber - anchorBlockNumber);
        if (joinCount == 0) {
            joinCount = 1;
        }

        for (int i = 0; i < joinCount; i++) {
            joinNextLine(editCursor);
        }
    } else {
        if (cursor.block().next().isValid()) {

            cursor.beginEditBlock();
            joinNextLine(cursor);
            cursor.endEditBlock();

            setTextCursor(cursor);
        }
    }
}

AtomicEditOperation::AtomicEditOperation(Qutepart *qutepart) : qutepart_(qutepart) {
    qutepart->textCursor().beginEditBlock();
}

AtomicEditOperation::~AtomicEditOperation() { qutepart_->textCursor().endEditBlock(); }

static QIcon getStatusIconImpl(const QString &name, QStyle::StandardPixmap backup) {
    auto static iconCache = QHash<QString, QIcon>();
    if (!iconCache.contains(name)) {
        if (QIcon::hasThemeIcon(name)) {
            iconCache[name] = QIcon::fromTheme(name);
        } else {
            iconCache[name] = qApp->style()->standardIcon(backup);
        }
    }
    return iconCache[name];
}

QIcon iconForStatus(int status) {
    if (status & WARNING_BIT) {
        return getStatusIconImpl("data-warning", QStyle::SP_MessageBoxWarning);
    }
    if (status & ERROR_BIT) {
        return getStatusIconImpl("data-error", QStyle::SP_MessageBoxCritical);
    }
    if (status & INFO_BIT) {
        return getStatusIconImpl("data-information", QStyle::SP_MessageBoxInformation);
    }
    /*
    if (status & OTHER_BIT) {
        return getStatusIconImpl("data-question", QStyle::SP_MessageBoxQuestion);
    }
    */
    return {};
}

void Qutepart::mousePressEvent(QMouseEvent *event) {
    if (event->modifiers() == Qt::AltModifier) {
        auto cursor = cursorForPosition(event->pos());
        auto exists = false;
        if (textCursor().position() == cursor.position()) {
            exists = true;
        }
        for (const auto &extraCursor : extraCursors) {
            if (extraCursor.position() == cursor.position()) {
                exists = true;
                break;
            }
        }
        if (!exists) {
            extraCursors.append(cursor);
            extraCursorsVisible_ = true;
            viewport()->repaint();
            extraCursorBlinkTimer_->stop();
            extraCursorBlinkTimer_->start();
            event->accept();
        }
    } else {
        if (!extraCursors.isEmpty()) {
            extraCursors.clear();
            extraCursorBlinkTimer_->stop();
            extraCursorsVisible_ = false;
            update();
        }
        QPlainTextEdit::mousePressEvent(event);
    }
}

void Qutepart::mouseReleaseEvent(QMouseEvent *event) {
    if (event->modifiers() != Qt::AltModifier) {
        QPlainTextEdit::mouseReleaseEvent(event);
    } else {
        event->accept();
    }
}

void Qutepart::toggleExtraCursorsVisibility() {
    extraCursorsVisible_ = !extraCursorsVisible_;
    viewport()->update();
}

QTextCursor Qutepart::applyOperationToAllCursors(
    std::function<void(QTextCursor &)> operation,
    std::function<bool(const QTextCursor &, const QTextCursor &)> sortOrderBeforeOp) {
    auto allCursors = extraCursors;
    allCursors.append(textCursor());

    if (sortOrderBeforeOp) {
        std::sort(allCursors.begin(), allCursors.end(), sortOrderBeforeOp);
    }

    AtomicEditOperation op(this);
    for (auto &currentCursor : allCursors) {
        operation(currentCursor);
    }

    std::sort(allCursors.begin(), allCursors.end(),
              [](const auto &a, const auto &b) { return a.position() < b.position(); });

    QTextCursor newMainCursor = allCursors.last();
    extraCursors.clear();
    for (int i = 0; i < allCursors.size() - 1; ++i) {
        extraCursors.append(allCursors[i]);
    }
    return newMainCursor;
}

void Qutepart::multipleCursorPaste() {
    if (extraCursors.isEmpty()) {
        QPlainTextEdit::paste();
        return;
    }

    auto allCursors = extraCursors;
    auto clipboardText = QApplication::clipboard()->text();
    auto lines = clipboardText.split('\n');

    allCursors.prepend(textCursor()); // Add main cursor first
    AtomicEditOperation op(this);
    if (lines.size() == allCursors.size()) {
        for (int i = 0; i < allCursors.size(); ++i) {
            auto cursor = allCursors[i];
            cursor.insertText(lines[i]);
        }
    } else {
        for (const auto &cursor : allCursors) {
            auto cur = cursor;
            cur.insertText(clipboardText);
        }
    }
}

void Qutepart::multipleCursorCopy() {
    auto allCursors = extraCursors;
    auto textToCopy = QString();

    allCursors.append(textCursor());
    std::sort(allCursors.begin(), allCursors.end(), [](const auto &a, const auto &b) {
        return a.block().blockNumber() < b.block().blockNumber();
    });
    auto anySelection = std::any_of(allCursors.begin(), allCursors.end(),
                                    [](const auto &cursor) { return cursor.hasSelection(); });

    if (anySelection) {
        auto selectedTexts = QStringList();
        for (const auto &cursor : allCursors) {
            if (cursor.hasSelection()) {
                selectedTexts << cursor.selectedText();
            }
        }
        textToCopy = selectedTexts.join('\n').replace(QChar(0x2029), '\n');
    } else {
        auto usedBlocks = QSet<int>();
        auto lines = QStringList();
        for (const auto &cursor : allCursors) {
            int blockNum = cursor.block().blockNumber();
            if (!usedBlocks.contains(blockNum)) {
                auto text = cursor.block().text();
                if (text.endsWith(QChar(0x2029))) {
                    text.chop(1);
                }
                lines << text;
                usedBlocks.insert(blockNum);
            }
        }
        textToCopy = lines.join('\n');
        if (!lines.isEmpty()) {
            textToCopy.append('\n');
        }
    }
    QApplication::clipboard()->setText(textToCopy);
}

void Qutepart::multipleCursorCut() {
    auto allCursors = extraCursors;
    allCursors.prepend(textCursor());

    std::sort(allCursors.begin(), allCursors.end(),
              [](const auto &a, const auto &b) { return a.position() < b.position(); });

    auto lines = QStringList();
    auto anySelection = false;
    for (auto &cursor : allCursors) {
        if (cursor.hasSelection()) {
            lines << cursor.selectedText();
            anySelection = true;
        } else {
            auto text = cursor.block().text();
            if (text.endsWith(QChar(0x2029))) {
                text.chop(1);
            }
            lines << text;
        }
    }
    auto textToCopy = lines.join('\n');
    if (!anySelection) {
        textToCopy.append('\n');
    }
    QApplication::clipboard()->setText(textToCopy);

    std::sort(allCursors.begin(), allCursors.end(), [](const auto &a, const auto &b) {
        return a.block().blockNumber() > b.block().blockNumber();
    });

    AtomicEditOperation op(this);
    auto minBlock = std::numeric_limits<int>::max();
    for (auto &cursor : allCursors) {
        if (cursor.hasSelection()) {
            minBlock =
                std::min(minBlock, std::min(cursor.block().blockNumber(),
                                            document()->findBlock(cursor.anchor()).blockNumber()));
            cursor.removeSelectedText();
        } else {
            minBlock = std::min(minBlock, cursor.block().blockNumber());
            cursor.select(QTextCursor::BlockUnderCursor);
            cursor.removeSelectedText();
        }
    }

    if (!anySelection) {
        if (document()->blockCount() == 0) {
            setTextCursor(QTextCursor(document()));
            return;
        }
        auto afterCutBlock = std::min(minBlock, document()->blockCount() - 1);
        auto targetBlock = document()->findBlockByNumber(afterCutBlock);
        if (targetBlock.isValid()) {
            QTextCursor caret(targetBlock);
            caret.movePosition(QTextCursor::StartOfBlock);
            setTextCursor(caret);
        }
    }
}

} // namespace Qutepart

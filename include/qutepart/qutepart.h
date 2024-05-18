/**
 * \file qutepart.h
 * \brief Main Qutepart header. Contains whole API.
 *
 * See also hl_factory.h if you need only syntax highlighter.
 */

#pragma once

#include <memory>

#include <QColor>
#include <QDebug>
#include <QPlainTextEdit>
#include <QSharedPointer>
#include <QSyntaxHighlighter>


namespace Qutepart {


/**
 * \enum IndentAlg
 * \brief Indentation algorithm.
 * Returned by ::chooseLanguage().
 *
 * Passed to ::Qutepart::Qutepart::setIndentAlgorithm()
 */
enum IndentAlg {
    /// Do not apply any algorithm. Default text editor behaviour.
    INDENT_ALG_NONE = 0,
    /// Insert to new lines indentation equal to previous line.
    INDENT_ALG_NORMAL,
    /// Algorithm for C-style languages where curly brackets are used to mark code blocks. C, C++, PHP, Java, JS, ...
    INDENT_ALG_CSTYLE,
    /// Lisp indentation.
    INDENT_ALG_LISP,
    /// Scheme indentation.
    INDENT_ALG_SCHEME,
    /// XML indentation.
    INDENT_ALG_XML,
    /// Python indentation.
    INDENT_ALG_PYTHON,
    /// Ruby indentation.
    INDENT_ALG_RUBY,
};


/**
 * Programming language ID and related information.
 *
 * This structure is returned by ::chooseLanguage()
 */
struct LangInfo {
public:
    LangInfo() = default;

    inline LangInfo(const QString& id, const QStringList& names, IndentAlg indentAlg):
        id(id),
        names(names),
        indentAlg(indentAlg)
    {};

    /// Check if the struct is valid (filled with meaningfull info)
    inline bool isValid() const {
        return ! id.isEmpty();
    }

    /// Internal unique language ID. Pass to ::Qutepart::Qutepart::setHighlighter()
    QString id;

    /// User readable language names
    QStringList names;

    /// Indenter algorithm for the language. Pass to ::Qutepart::Qutepart::setIndentAlgorithm()
    IndentAlg indentAlg;

};

/**
 * Choose language by available parameters.
 * First parameters have higher priority.
 * Returns `QString()` if can not detect the language.
 *
 * Fill as much parameters as you can. Set `QString()` for unknown parameters.
 *
 * \param mimeType The file MIME type. i.e. ``text/html``
 * \param languageName The language name as written in the <a href="https://github.com/andreikop/qutepart-cpp/blob/master/src/hl/language_db_generated.cpp">language DB</a>
 * \param sourceFilePath The path to the file which is edited.
 * \param firstLine Contents of the first line of the file which is going to be edited.
 */
LangInfo chooseLanguage(
    const QString& mimeType=QString(),
    const QString& languageName=QString(),
    const QString& sourceFilePath=QString(),
    const QString& firstLine=QString());

class Indenter;
class BracketHighlighter;
class LineNumberArea;
class MarkArea;
class Completer;

/**
 * Document line.
 *
 * A convenience class to programmatically edit the document
 */
class Line {
public:
    Line(const QTextBlock& block);

    /// Get line text
    QString text() const;

    /// Get line length not including EOL symbol
    int length() const;

    /// Remove the line from the document
    void remove(int pos, int count);

private:
    QTextBlock block_;
};

/** STL-compatible iterator implementation to work with document lines (blocks)
 *
 * Returns ::Qutepart::Line objects
 */
class LineIterator {
public:
    LineIterator(const QTextBlock& block);

    bool operator!=(const LineIterator& other);
    bool operator==(const LineIterator& other);
    LineIterator operator++();
    Line operator*();

private:
    QTextBlock block_;
};

/** A convenience class which provides high level interface to work with
 * the document lines.
 *
 * Returned by ::Qutepart::Qutepart::lines()
 *
 * `Lines` is a performance-effective document representation.
 * Getting whole text of document with `QPlainTextEdit::toPlainText()`` requires a lot of memory allocations and copying. This class accesses the text line by line without copying whole document.
*/

class Lines {
public:
    Lines(QTextDocument* document);

    /// Line count in the document
    int count() const;

    /// Get line by index.
    Line at(int index) const;

    /// `begin()` method for STL iteration support
    LineIterator begin();

    /// `end()` method for STL iteration support
    LineIterator end();

    /// First line of the document
    Line first() const;

    /// Last line of the document
    Line last() const;

    /// Append line to the end of the document.
    void append(const QString& lineText);

    // Remove and return line at number. Return the text wo \n
    QString popAt(int lineNumber);

    // Insert at given line number one or more lines.
    // The text shoud be \n-separated. \n at end is interpreted as empty line.
    void insertAt(int lineNumber, const QString& text);
private:
    QTextDocument* document_;
};


/** Cursor position
 *
 * A convenience class, which is more friendly than low level QTextCursor API.
 *
 * Returned by ::Qutepart::Qutepart::textCursorPosition()
 */
struct TextCursorPosition {
    TextCursorPosition(int line_, int column_):
        line(line_),
        column(column_)
    {}

    friend bool operator== (const TextCursorPosition& a, const TextCursorPosition& b) {
        return a.line == b.line && a.column == b.column;
    }

    /// Current line. First line is 0.
    int line;

    /// Current column. First column is 0.
    int column;
};


/**
  Code editor widget
*/
class Qutepart: public QPlainTextEdit {
    Q_OBJECT

public:
    Qutepart(QWidget *parent = nullptr, const QString &text = QString());

    // Not copyable or movable
    Qutepart(const Qutepart&) = delete;
    Qutepart& operator=(const Qutepart&) = delete;
    Qutepart(Qutepart&&) = delete;
    Qutepart& operator=(Qutepart&&) = delete;

    virtual ~Qutepart();

    /// High-performance access to document lines. See ::Qutepart::Lines
    Lines lines() const;

    /**
     * Set highlighter. Use `Qutepart::chooseLanguage()` to choose the language
     *
     * \param languageId Language name. See Qutepart::LangInfo::id.
     */
    void setHighlighter(const QString& languageId);

    /**
     * Set indenter algorithm. Use `Qutepart::chooseLanguage()` to choose the algorithm.
     *
     * \param indentAlg Algorithm name. See Qutepart::LangInfo::indentAlg.
     */
    void setIndentAlgorithm(IndentAlg indentAlg);

    /// Convenience method to get text cursor position.
    TextCursorPosition textCursorPosition() const;

    /// Go to specified line and column. First line and first column have index 0
    void goTo(int line, int column=0);
    /// Go to text position specified by ::Qutepart::TextCursorPosition
    void goTo(const TextCursorPosition& pos);

    /// Indent current line using current smart indentation algorithm
    void autoIndentCurrentLine();

    /// Use Tabs instead of spaces for indentation
    bool indentUseTabs() const;
    /// Use Tabs instead of spaces for indentation
    void setIndentUseTabs(bool);

    /// Indentation width. Count of inserted spaces, Tab symbol display width
    int indentWidth() const;
    /// Indentation width. Count of inserted spaces, Tab symbol display width
    void setIndentWidth(int);

    /// Visual option. Draw indentation symbols.
    bool drawIndentations() const;
    /// Visual option. Draw indentation symbols.
    void setDrawIndentations(bool);

    /// Visual option. Draw any whitespace symbol.
    bool drawAnyWhitespace() const;
    /// Visual option. Draw any whitespace symbol.
    void setDrawAnyWhitespace(bool);

    /// Visual option. Draw incorrent indentation. i.e. at end of line or Tab after spaces.
    bool drawIncorrectIndentation() const;
    /// Visual option. Draw incorrent indentation. i.e. at end of line or Tab after spaces.
    void setDrawIncorrectIndentation(bool);

    /// Visual option. Draw solid line length marker (usually after column 80)
    bool drawSolidEdge() const;
    /// Visual option. Draw solid line length marker (usually after column 80)
    void setDrawSolidEdge(bool);

    /// Visual option. Column on which line lendth marker is drawn.
    int lineLengthEdge() const;
    /// Visual option. Column on which line lendth marker is drawn.
    void setLineLengthEdge(int);

    /// Visual option. Color of line lendth edge.
    QColor lineLengthEdgeColor() const;
    /// Visual option. Color of line lendth edge.
    void setLineLengthEdgeColor(QColor);

    /// Visual option. Color of current line highlighting. `QColor()` if disabled.
    QColor currentLineColor() const;
    /// Visual option. Color of current line highlighting. `QColor()` if disabled.
    void setCurrentLineColor(QColor);

    bool bracketHighlightingEnabled() const;
    void setBracketHighlightingEnabled(bool value);

    bool lineNumbersVisible() const;
    void setLineNumbersVisible(bool value);

    // Autocompletion
    void setCompletionEnabled(bool);
    bool completionEnabled() const;

    void setCompletionThreshold(int);
    int completionThreshold() const;

    // Actions
    QAction* increaseIndentAction() const;
    QAction* decreaseIndentAction() const;

    QAction* toggleBookmarkAction() const;
    QAction* prevBookmarkAction() const;
    QAction* nextBookmarkAction() const;

    QAction* invokeCompletionAction() const;

    QAction* scrollDownAction() const;
    QAction* scrollUpAction() const;

    QAction* duplicateSelectionAction() const;

    QAction* moveLineUpAction() const;
    QAction* moveLineDownAction() const;

    QAction* deleteLineAction() const;

    QAction* cutLineAction() const;
    QAction* copyLineAction() const;
    QAction* pasteLineAction() const;

    QAction* insertLineAboveAction() const;
    QAction* insertLineBelowAction() const;

    QAction* joinLinesAction() const;

    /// Zoom In the document by scaling fonts
    QAction* zoomInAction() const;
    /// Zoom Out the document by scaling fonts
    QAction* zoomOutAction() const;

    // Convenience functions
    void resetSelection();

protected:
    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;
    void paintEvent(QPaintEvent *event) override;
    void changeEvent(QEvent *event) override;

private:
    void initActions();
    QAction* createAction(const QString& text, QKeySequence shortcut,
                          const QString& iconFileName,
                          std::function<void()> const& handler);

    // whitespace and edge drawing
    void drawIndentMarkersAndEdge(const QRect& rect);
    void drawIndentMarker(QPainter* painter, QTextBlock block, int column);
    void drawEdgeLine(QPainter* painter, QTextBlock block, int edgePos);
    void drawWhiteSpace(QPainter* painter, QTextBlock block, int column, QChar ch);
    int effectiveEdgePos(const QString& text);
    void chooseVisibleWhitespace(const QString& text, QVector<bool>* result);

    QTextEdit::ExtraSelection currentLineExtraSelection() const;

    void setSolidEdgeGeometry();
    void resizeEvent(QResizeEvent* event) override;

    void updateTabStopWidth();

    QRect cursorRect(QTextBlock block, int column, int offset) const;
    void gotoBlock(const QTextBlock& block);

    void indentBlock(const QTextBlock& block, bool withSpace) const;
    void unIndentBlock(const QTextBlock& block, bool withSpace) const;
    void changeSelectedBlocksIndent(bool increase, bool withSpace);

    void scrollByOffset(int offset);

    void duplicateSelection();
    void moveSelectedLines(int offsetLines);

    void deleteLine();

    void cutLine();
    void copyLine();
    void pasteLine();

    void insertLineAbove();
    void insertLineBelow();

private slots:
    void updateViewport();
    void updateExtraSelections();

    void onShortcutHome(QTextCursor::MoveMode moveMode);

    void onShortcutToggleBookmark();
    void onShortcutPrevBookmark();
    void onShortcutNextBookmark();

    void joinNextLine(QTextCursor& cursor);
    void onShortcutJoinLines();

private:
    QSharedPointer<QSyntaxHighlighter> highlighter_;
    std::unique_ptr<Indenter> indenter_;
    std::unique_ptr<BracketHighlighter> bracketHighlighter_;
    std::unique_ptr<LineNumberArea> lineNumberArea_;
    std::unique_ptr<MarkArea> markArea_;
    std::unique_ptr<Completer> completer_;

    bool drawIndentations_;
    bool drawAnyWhitespace_;
    bool drawIncorrectIndentation_;
    bool drawSolidEdge_;
    int lineLengthEdge_;
    QColor lineLengthEdgeColor_;
    QColor currentLineColor_;

    bool completionEnabled_;
    int completionThreshold_;

    QWidget* solidEdgeLine_;
    int totalMarginWidth_;

    QAction* homeAction_;  // private, not API
    QAction* homeSelectAction_;  // private, not API

    QAction* increaseIndentAction_;
    QAction* decreaseIndentAction_;

    QAction* toggleBookmarkAction_;
    QAction* prevBookmarkAction_;
    QAction* nextBookmarkAction_;

    QAction* invokeCompletionAction_;

    QAction* scrollDownAction_;
    QAction* scrollUpAction_;

    QAction* duplicateSelectionAction_;

    QAction* moveLineUpAction_;
    QAction* moveLineDownAction_;
    QAction* deleteLineAction_;

    QAction* cutLineAction_;
    QAction* copyLineAction_;
    QAction* pasteLineAction_;

    QAction* insertLineAboveAction_;
    QAction* insertLineBelowAction_;

    QAction* joinLinesAction_;

    QAction* zoomInAction_;
    QAction* zoomOutAction_;

    friend class LineNumberArea;
    friend class MarkArea;
};

/**
A helper class which allows to group edit operations on Qutepart using RAII approach.
Operations are undo-redoble as a single change.
Example:

    {
        AtomicEditOperation op(qutepart);
        qutepart.lines().insertAt(3, "line three");
        qutepart.lines().insertAt(4, "line four");
    }
 */
class AtomicEditOperation {
public:
    AtomicEditOperation(Qutepart* qutepart);
    ~AtomicEditOperation();

private:
    Qutepart* qutepart_;
};


} // namespace Qutepart

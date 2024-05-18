#include <QRegularExpression>
#include <QAbstractItemModel>
#include <QListView>
#include <QDebug>

#include "completer.h"
#include "qutepart.h"
#include "html_delegate.h"

/* Autocompletion widget and logic
 */


namespace Qutepart {

namespace {

const QString wordPattern("\\w+");
const QRegularExpression wordRegExp(wordPattern);
const QRegularExpression wordAtEndRegExp(wordPattern + '$');
const QRegularExpression wordAtStartRegExp("^" + wordPattern);

// Maximum count of words, for which completion will be shown. Ignored, if completion invoked manually.
const int MAX_VISIBLE_WORD_COUNT = 256;

const int MAX_VISIBLE_ROWS = 20;  // no any technical reason, just for better UI

const int WIDGET_BORDER_MARGIN = 5;
const int SCROLLBAR_WIDTH = 30;  // just a guess

}  // anonymous namespace

/* QAbstractItemModel implementation for a list of completion variants

words attribute contains all words
canCompleteText attribute contains text, which may be inserted with tab
*/
class CompletionModel: public QAbstractItemModel {
    Q_OBJECT
public:
    CompletionModel(const QSet<QString>& wordSet):
        wordSet_(wordSet)
    {}

    //Set model information
    void setData(const QString& wordBeforeCursor, const QString wholeWord) {
        typedText_ = wordBeforeCursor;
        words_ = makeListOfCompletions(wordBeforeCursor, wholeWord);
        QString commonStart = commonWordStart(words_);
        canCompleteText_ = commonStart.mid(wordBeforeCursor.length());

        emit(layoutChanged());
    }

    bool hasWords() const {
        return ! words_.isEmpty();
    }

    const QVector<QString>& words() const {
        return words_;
    }

    bool tooManyWords() const {
        return words_.length() > MAX_VISIBLE_WORD_COUNT;
    }

    // QAbstractItemModel method implementation
    QVariant data(const QModelIndex& index, int role) const override {
        if (role == Qt::DisplayRole && index.row() < words_.length()) {
            QString text = words_[index.row()];
            QString typed = text.left(typedText_.length());
            QString canComplete = text.mid(typedText_.length(), canCompleteText_.length());
            QString rest = text.mid(typedText_.length() + canCompleteText_.length());
            if ( ! canComplete.isEmpty()) {
                // NOTE foreground colors are hardcoded, but I can't set background color of selected item (Qt bug?)
                // might look bad on some color themes
                return QString(
                        "<html>"
                            "%1"
                            "<font color=\"#e80000\">%2</font>"
                            "%3"
                        "</html>"
                            ).arg(typed, canComplete, rest);
            } else {
                return typed + rest;
            }
        } else {
            return QVariant();
        }
    }

    // QAbstractItemModel method implementation
    int rowCount(const QModelIndex& = QModelIndex()) const override {
        return words_.length();
    }

    //Get current typed text
    QString typedText() const {
        return typedText_;
    }

    /*Get common start of all words.
    i.e. for ['blablaxxx', 'blablayyy', 'blazzz'] common start is 'bla'
    TODO optimize performance?
    */
    QString commonWordStart(QVector<QString>& words) const {
        if (words.isEmpty()) {
            return "";
        }

        QString firstWord = words[0];
        for(int chIndex = 0; chIndex < firstWord.length(); chIndex++) {
            QChar ch = firstWord[chIndex];
            for(int wordIndex = 1; wordIndex < words.length(); wordIndex++) {
                if (words[wordIndex][chIndex] != ch) {
                    return firstWord.left(chIndex);
                }
            }
        }

        return firstWord;
    }

    // Make list of completions, which shall be shown
    QVector<QString> makeListOfCompletions(const QString& wordBeforeCursor, const QString& wholeWord) const {
        QVector<QString> result;
        foreach(const QString& word, wordSet_) {
            if (word.startsWith(wordBeforeCursor) &&
                word != wholeWord) {
                result << word;
            }
        }

        std::sort(result.begin(), result.end());

        return result;
    }

    // Trivial QAbstractItemModel methods implementation
    Qt::ItemFlags flags(const QModelIndex&) const override {
        return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
    }

    QVariant headerData(int , Qt::Orientation , int = Qt::DisplayRole) const override {
        return QVariant();
    }

    int columnCount(const QModelIndex&) const override {
        return 1;
    }

    QModelIndex index(int row, int column, const QModelIndex& = QModelIndex()) const override {
        return createIndex(row, column);
    }

    QModelIndex parent(const QModelIndex&) const override {
        return QModelIndex();
    }

    QString canCompleteText() const {
        return canCompleteText_;
    }

signals:
    void layoutChanged();

private:
    const QSet<QString>& wordSet_;
    QString typedText_;
    QVector<QString> words_;
    QString canCompleteText_;
};

// Completion list widget
class CompletionList: public QListView {
    Q_OBJECT
public:
    CompletionList(Qutepart* qpart, CompletionModel* model):
            QListView(qpart->viewport()),
            qpart_(qpart),
            completionModel_(model),
            selectedIndex_(-1) {
        // ensure good selected item background on Windows
        QPalette newPalette = palette();
        newPalette.setColor(QPalette::Inactive, QPalette::Highlight,
                            newPalette.color(QPalette::Active, QPalette::Highlight));
        setPalette(newPalette);

        setAttribute(Qt::WA_DeleteOnClose);

        setItemDelegate(new HTMLDelegate(this));

        setFont(qpart_->font());

        setCursor(QCursor(Qt::PointingHandCursor));
        setFocusPolicy(Qt::NoFocus);

        setModel(model);

        // if cursor moved, we shall close widget, if its position (and model) hasn't been updated
        closeIfNotUpdatedTimer_.setInterval(200);
        closeIfNotUpdatedTimer_.setSingleShot(true);

        connect(&closeIfNotUpdatedTimer_, &QTimer::timeout, this, &CompletionList::afterCursorPositionChanged);

        qpart_->installEventFilter(this);

        connect(qpart, &Qutepart::cursorPositionChanged, this, &CompletionList::onCursorPositionChanged);

        connect(this, &QListView::clicked, [this](QModelIndex index){emit(this->itemSelected(index.row()));});

        updateGeometry();
        show();

        qpart_->setFocus();
    }

    ~CompletionList() {
        closeIfNotUpdatedTimer_.stop();
        qpart_->removeEventFilter(this);
    }

    /* QWidget.sizeHint implementation
    Automatically resizes the widget according to rows count

    FIXME very bad algorithm. Remove all this margins, if you can
    */
    QSize sizeHint() const override {
        int width = -1;
        foreach(const QString& word, completionModel_->words()) {
            int wordWidth = fontMetrics().width(word);
            if (wordWidth > width) {
                width = wordWidth;
            }
        }

        width = width * 1.4;  // FIXME bad hack. invent better formula
        width += 30;  // margin

        // drawn with scrollbar without +2. I don't know why
        int rowCount = std::min(model()->rowCount(), MAX_VISIBLE_ROWS);
        int height = sizeHintForRow(0) * (rowCount + 0.5);  // + 0.5 row margin

        return QSize(width, height);
    }

    // QWidget::minimumSizeHint implementation
    int minimumHeight() const {
        return sizeHintForRow(0) * 1.5;  // + 0.5 row margin
    }

    CompletionModel* completionModel() const {
        return completionModel_;
    }

    //Move widget to point under cursor
    void updateGeometry() {
        QSize hint = sizeHint();
        int width = hint.width();
        int height = hint.height();

        QRect cursorRect = qpart_->QPlainTextEdit::cursorRect(qpart_->textCursor());
        QSize parentSize = parentWidget()->size();

        int spaceBelow = parentSize.height() - cursorRect.bottom() - WIDGET_BORDER_MARGIN;
        int spaceAbove = cursorRect.top() - WIDGET_BORDER_MARGIN;
        int xPos = -1;
        int yPos = -1;

        if (height <= spaceBelow ||
            spaceBelow > spaceAbove) {
            yPos = cursorRect.bottom();
            if (height > spaceBelow &&
                spaceBelow > minimumHeight()) {
                height = spaceBelow;
                width = width + SCROLLBAR_WIDTH;
            }
        } else {
            if (height > spaceAbove &&
                spaceAbove > minimumHeight()) {
                height = spaceAbove;
                width = width + SCROLLBAR_WIDTH;
            }
            yPos = std::max(3, cursorRect.top() - height);
        }

        xPos = cursorRect.right() - horizontalShift();

        if (xPos + width + WIDGET_BORDER_MARGIN > parentSize.width()) {
            xPos = std::max(3, parentSize.width() - WIDGET_BORDER_MARGIN - width);
        }

        setGeometry(xPos, yPos, width, height);
        closeIfNotUpdatedTimer_.stop();
    }

private:

    /* List should be plased such way, that typed text in the list is under
    typed text in the editor
    */
    int horizontalShift() const {
        int strangeAdjustment = 2;  // I don't know why. Probably, won't work on other systems and versions
        return fontMetrics().width(completionModel_->typedText()) + strangeAdjustment;
    }

    /* Cursor position changed. Schedule closing.
    Timer will be stopped, if widget position is being updated
    */
    void onCursorPositionChanged() {
        closeIfNotUpdatedTimer_.start();
    }

    // Widget position hasn't been updated after cursor position change, close widget
    void afterCursorPositionChanged() {
        emit(closeMe());
    }

    /* Catch events from qpart
    Move selection, select item, or close themselves
    */
    bool eventFilter(QObject* /*object*/, QEvent* event) override {
        if (event->type() == QEvent::KeyPress &&
            dynamic_cast<QKeyEvent*>(event)->modifiers() == Qt::NoModifier) {
            switch(dynamic_cast<QKeyEvent*>(event)->key()) {
                case Qt::Key_Escape:
                    emit(closeMe());
                    return true;
                case Qt::Key_Down:
                    if (selectedIndex_ + 1 < model()->rowCount()) {
                        selectItem(selectedIndex_ + 1);
                    }
                    return true;
                case Qt::Key_Up:
                    if (selectedIndex_ - 1 >= 0) {
                        selectItem(selectedIndex_ - 1);
                    }
                    return true;
                case Qt::Key_Enter:
                case Qt::Key_Return:
                    if (selectedIndex_ != -1) {
                        emit(itemSelected(selectedIndex_));
                        return true;
                    }
                    return false;
                case Qt::Key_Tab:
                    emit(tabPressed());
                    return true;
            }
        } else if (event->type() == QEvent::FocusOut) {
            emit(closeMe());
        }

        return false;
    }

    // Select item in the list
    void selectItem(int index) {
        selectedIndex_ = index;
        setCurrentIndex(model()->index(index, 0));
    }

signals:
    void closeMe();
    void itemSelected(int);
    void tabPressed();

private:
    Qutepart* qpart_;
    CompletionModel* completionModel_;  // equals to model() but is typed
    int selectedIndex_;
    QTimer closeIfNotUpdatedTimer_;
};

// Object listens Qutepart widget events, computes and shows autocompletion lists

Completer::Completer(Qutepart* qpart):
        QObject(qpart),
        qpart_(qpart),
        widget_(nullptr),
        completionOpenedManually_(false) {
    connect(qpart, &Qutepart::textChanged, this, &Completer::onTextChanged);
    connect(qpart->document(), &QTextDocument::modificationChanged, this, &Completer::onModificationChanged);

    updateWordSetTimer_.setSingleShot(true);
    connect(&updateWordSetTimer_, &QTimer::timeout, this, &Completer::updateWordSet);
}

// Object deleted. Cancel timer
Completer::~Completer() {
    updateWordSetTimer_.stop();
}

void Completer::setKeywords(const QSet<QString>& keywords) {
    keywords_ = keywords;
    updateWordSet();
}

void Completer::setCustomCompletions(const QSet<QString>& wordSet) {
    customCompletions_ = wordSet;
}

bool Completer::isVisible() const {
    return widget_ != nullptr;
}

// Text in the qpart changed. Update word set
void Completer::onTextChanged() {
    updateWordSetTimer_.start();
    updateWordSet();
}

void Completer::onModificationChanged(bool modified) {
    if ( ! modified) {
        closeCompletion();
    }
}

// Make a set of words, which shall be completed, from text
void Completer::updateWordSet() {
    wordSet_ = keywords_.unite(customCompletions_);

    // TODO check for timeout
    for(const Line& line: qpart_->lines()) {
        QRegularExpressionMatchIterator it = wordRegExp.globalMatch(line.text());

        while (it.hasNext()) {
            QRegularExpressionMatch match = it.next();
            wordSet_ += match.captured();
        }
    }
}

// Invoke completion manually
void Completer::invokeCompletion() {
    if (invokeCompletionIfAvailable(true)) {
        completionOpenedManually_ = true;
    }
}

bool Completer::shouldShowModel(CompletionModel* model, bool forceShow) {
    if ( ! model->hasWords()) {
        return false;
    }

    return forceShow ||
           ( ! model->tooManyWords());
}

void Completer::createWidget(CompletionModel* model) {
    widget_ = std::make_unique<CompletionList>(qpart_, model);
    connect(widget_.get(), &CompletionList::closeMe, this, &Completer::closeCompletion);
    connect(widget_.get(), &CompletionList::itemSelected, this, &Completer::onCompletionListItemSelected);
    connect(widget_.get(), &CompletionList::tabPressed, this, &Completer::onCompletionListTabPressed);
}

/* Invoke completion, if available. Called after text has been typed in qpart
Returns True, if invoked
*/
bool Completer::invokeCompletionIfAvailable(bool requestedByUser) {
    if (qpart_->completionEnabled() && (! wordSet_.isEmpty())) {
        QString wordBeforeCursor = getWordBeforeCursor();
        QString wholeWord = wordBeforeCursor + getWordAfterCursor();

        bool forceShow = requestedByUser || completionOpenedManually_;
        if ( ! wordBeforeCursor.isEmpty()) {
            if (wordBeforeCursor.length() >= qpart_->completionThreshold() || forceShow) {
                if (widget_ == nullptr) {
                    CompletionModel* model = new CompletionModel(wordSet_);
                    model->setData(wordBeforeCursor, wholeWord);
                    if (shouldShowModel(model, forceShow)) {
                        createWidget(model);
                        return true;
                    }
                } else {
                    widget_->completionModel()->setData(wordBeforeCursor, wholeWord);
                    if (shouldShowModel(widget_->completionModel(), forceShow)) {
                        widget_->updateGeometry();

                        return true;
                    }
                }
            }
        }
    }

    closeCompletion();
    return false;
}

/* Close completion, if visible.
Delete widget
*/
void Completer::closeCompletion() {
    if (bool(widget_)) {
        widget_.reset();
        completionOpenedManually_ = false;
    }
}

// Get word, which is located before cursor
QString Completer::getWordBeforeCursor() const {
    QTextCursor cursor = qpart_->textCursor();
    QString textBeforeCursor = cursor.block().text().right(cursor.positionInBlock());
    QRegularExpressionMatch match = wordAtEndRegExp.match(textBeforeCursor);
    if (match.hasMatch()) {
        return match.captured();
    } else {
        return QString();
    }
}

// Get word, which is located before cursor
QString Completer::getWordAfterCursor() const {
    QTextCursor cursor = qpart_->textCursor();
    QString textAfterCursor = cursor.block().text().mid(cursor.positionInBlock());
    QRegularExpressionMatch match = wordAtStartRegExp.match(textAfterCursor);
    if (match.hasMatch()) {
        return match.captured();
    } else {
        return QString();
    }
}

// Item selected. Insert completion to editor
void Completer::onCompletionListItemSelected(int index) {
    CompletionModel* model = widget_->completionModel();
    QString selectedWord = model->words()[index];
    QString textToInsert = selectedWord.mid(model->typedText().length());
    qpart_->textCursor().insertText(textToInsert);
    closeCompletion();
}

/* Tab pressed on completion list
Insert completable text, if available
*/
void Completer::onCompletionListTabPressed() {
    QString canCompleteText = widget_->completionModel()->canCompleteText();
    if ( ! canCompleteText.isEmpty()) {
        qpart_->textCursor().insertText(canCompleteText);
        invokeCompletionIfAvailable(false);
    }
}

}  // namespace Qutepart

#include "completer.moc"

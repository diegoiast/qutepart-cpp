#include "text_to_match.h"


namespace Qutepart {

TextToMatch::TextToMatch(
        const QString& text,
        const QString& deliminatorSet,
        const QStringList& contextData):
    currentColumnIndex(0),
    wholeLineText(text),
    text(wholeLineText.leftRef(wholeLineText.length())),
    textLength(text.length()),
    firstNonSpace(true), // copy-paste from Py code
    isWordStart(true), // copy-paste from Py code
    contextData(&contextData),
    mDeliminatorSet(deliminatorSet)
{
    findWord();
}

void TextToMatch::findWord() {
    if (currentColumnIndex > 0) {
        QChar prevChar = wholeLineText[currentColumnIndex - 1];
        if ( ! mDeliminatorSet.contains(prevChar)) {
            word = QString::null;
            return;
        }
    }

    int wordEndIndex = 0;
    for(; wordEndIndex < text.length(); wordEndIndex++) {
        if (mDeliminatorSet.contains(text.at(wordEndIndex))) {
            break;
        }
    }
    if (wordEndIndex != 0) {
        word = text.left(wordEndIndex).toString();
    }
}

void TextToMatch::shiftOnce() {
    QChar prevChar = text.at(0);
    firstNonSpace = firstNonSpace && prevChar.isSpace();
    isWordStart = prevChar.isSpace() || mDeliminatorSet.contains(prevChar);

    currentColumnIndex++;
    text = text.right(text.length() - 1);
    textLength--;

    findWord();
}

void TextToMatch::shift(int count) {
    for(int i = 0; i < count; i++) {
        QChar prevChar = text.at(i);
        firstNonSpace = firstNonSpace && prevChar.isSpace();
        isWordStart = prevChar.isSpace() || mDeliminatorSet.contains(prevChar);
    }

    currentColumnIndex += count;
    text = text.right(text.length() - count);
    textLength -= count;
    findWord();
}

bool TextToMatch::isEmpty() const {
    return text.isEmpty();
}

void TextToMatch::setCurrentContextKeywordDeliminators(const QString& deliminatorSet) {
    /*
        Keyword deliminators are language specific.
        When one language is included to other, it is necessary to re-find word
        with new deliminators
     */
    if (deliminatorSet != mDeliminatorSet) {
        mDeliminatorSet = deliminatorSet;
        findWord();
    }
}

};
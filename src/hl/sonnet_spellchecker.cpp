#include "sonnet_spellchecker.h"
#include "syntax_highlighter.h"
#include "text_type.h"

#include <QTextCharFormat>

#include "sonnet_gen/Sonnet/Speller"
#include "sonnet_gen/Sonnet/Tokenizer"

namespace Qutepart {

SonnetSpellChecker::SonnetSpellChecker() {
    speller_ = new Sonnet::Speller();
    wordTokenizer_ = new Sonnet::WordTokenizer();
}

SonnetSpellChecker::~SonnetSpellChecker() {
    delete speller_;
    delete wordTokenizer_;
}

void SonnetSpellChecker::spellCheck(const QTextBlock &block, SyntaxHighlighter *highlighter) {
    const QString text = block.text();

    if (text.isEmpty() || !speller_->isValid()) {
        return;
    }

    wordTokenizer_->setBuffer(text);
    while (wordTokenizer_->hasNext()) {
        Sonnet::Token word = wordTokenizer_->next();
        if (!wordTokenizer_->isSpellcheckable()) {
            continue;
        }
        if (!isSpellCheckable(block, word.position())) {
            continue;
        }

        if (speller_->isMisspelled(word.toString())) {
            qDebug() << "  MISSPELLED:" << word.toString();
            QTextCharFormat format;
            format.setFontUnderline(true);
            format.setUnderlineStyle(QTextCharFormat::SpellCheckUnderline);
            format.setUnderlineColor(Qt::red);
            highlighter->addBlockFormat(word.position(), word.length(), format);
        }
    }
}

} // namespace Qutepart

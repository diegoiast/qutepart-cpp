#include "sonnet_spellchecker.h"
#include "text_block_user_data.h"

#include <QTextCharFormat>

#include "sonnet_gen/Sonnet/Speller"
#include "sonnet_gen/Sonnet/Tokenizer"
#include "sonnet_gen/Sonnet/LanguageFilter"

namespace Qutepart {

void SonnetLanguageCache::invalidate(int pos) {
    auto it = languages.begin();
    while (it != languages.end()) {
        if (it.key().first + it.key().second >= pos) {
            it = languages.erase(it);
        } else {
            ++it;
        }
    }
}

QString SonnetLanguageCache::languageAtPos(int pos) const {
    for (auto it = languages.begin(); it != languages.end(); ++it) {
        if (it.key().first <= pos && it.key().first + it.key().second >= pos) {
            return it.value();
        }
    }
    return QString();
}

SonnetSpellChecker::SonnetSpellChecker()
    : QSyntaxHighlighter(static_cast<QObject*>(nullptr)) {
    speller_ = new Sonnet::Speller();
    wordTokenizer_ = new Sonnet::WordTokenizer();
    languageFilter_ = new Sonnet::LanguageFilter(new Sonnet::SentenceTokenizer());
}

SonnetSpellChecker::~SonnetSpellChecker() {
    delete speller_;
    delete wordTokenizer_;
    delete languageFilter_;
}

void SonnetSpellChecker::highlightBlock(const QString &text) {
    Q_UNUSED(text);
    spellCheck(currentBlock());
}

void SonnetSpellChecker::spellCheck(const QTextBlock &block) {
    QString text = block.text();
    if (text.isEmpty() || !speller_->isValid()) return;

    auto data = dynamic_cast<TextBlockUserData *>(block.userData());
    if (!data) return;

    SonnetLanguageCache *cache = nullptr;
    if (data->additionalData.contains("sonnet")) {
        cache = dynamic_cast<SonnetLanguageCache *>(data->additionalData["sonnet"]);
    }

    if (!cache) {
        cache = new SonnetLanguageCache();
        data->additionalData["sonnet"] = cache;
    }

    languageFilter_->setBuffer(text);
    const bool autodetect = speller_->testAttribute(Sonnet::Speller::AutoDetectLanguage);

    while (languageFilter_->hasNext()) {
        Sonnet::Token sentence = languageFilter_->next();
        if (autodetect) {
            QString lang;
            QPair<int, int> spos(sentence.position(), sentence.length());
            if (cache->languages.contains(spos)) {
                lang = cache->languages.value(spos);
            } else {
                lang = languageFilter_->language();
                if (!languageFilter_->isSpellcheckable()) lang.clear();
                cache->languages[spos] = lang;
            }
            if (lang.isEmpty()) continue;
            speller_->setLanguage(lang);
        }

        wordTokenizer_->setBuffer(sentence.toString());
        int offset = sentence.position();
        while (wordTokenizer_->hasNext()) {
            Sonnet::Token word = wordTokenizer_->next();
            if (!wordTokenizer_->isSpellcheckable()) continue;

            if (speller_->isMisspelled(word.toString())) {
                QTextCharFormat format;
                format.setFontUnderline(true);
                format.setUnderlineStyle(QTextCharFormat::SpellCheckUnderline);
                format.setUnderlineColor(Qt::red);
                setFormat(word.position() + offset, word.length(), format);
            }
        }
    }
}

} // namespace Qutepart

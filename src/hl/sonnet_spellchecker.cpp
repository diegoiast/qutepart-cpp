#include "sonnet_spellchecker.h"
#include "syntax_highlighter.h"
#include "text_type.h"

#include <QDir>
#include <QFile>
#include <QLoggingCategory>
#include <QSet>
#include <QStandardPaths>
#include <QTextCharFormat>
#include <QtLogging>

#include "sonnet_gen/Sonnet/Speller"
#include "sonnet_gen/Sonnet/Tokenizer"

namespace Qutepart {

static QString findHunspellDictDir(const QString &lang) {
    QStringList searchDirs =
        QStandardPaths::locateAll(QStandardPaths::GenericDataLocation,
                                  QStringLiteral("hunspell"),
                                  QStandardPaths::LocateDirectory);
    searchDirs << QStringLiteral("/usr/share/hunspell/")
               << QStringLiteral("/usr/share/myspell/dicts/");
    for (const QString &dir : std::as_const(searchDirs)) {
        if (QFile::exists(QDir(dir).filePath(lang + QStringLiteral(".dic")))) {
            return dir;
        }
    }
    return {};
}

SonnetSpellChecker::SonnetSpellChecker() {
    QLoggingCategory::setFilterRules(QStringLiteral("sonnet.*=false"));
    speller_ = new Sonnet::Speller();
    wordTokenizer_ = new Sonnet::WordTokenizer();
    if (speller_->isValid()) {
        const QString lang = speller_->language();
        const QString dictDir = findHunspellDictDir(lang);
        const QString userDict = QDir::home().filePath(QStringLiteral(".hunspell_") + lang);
        qInfo() << "Spell checker: language=" << lang
                << "dict=" << (dictDir.isEmpty() ? QStringLiteral("(not found)") : dictDir)
                << "personal=" << userDict;
    } else {
        qWarning() << "Spell checker: no valid dictionary found";
    }
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
            QTextCharFormat format;
            format.setFontUnderline(true);
            format.setUnderlineStyle(QTextCharFormat::SpellCheckUnderline);
            format.setUnderlineColor(Qt::red);
            highlighter->addBlockFormat(word.position(), word.length(), format);
        }
    }
}

bool SonnetSpellChecker::isMisspelled(const QString &word) {
    if (!speller_->isValid() || ignoredWords_.contains(word)) {
        return false;
    }
    return speller_->isMisspelled(word);
}

void SonnetSpellChecker::ignoreWord(const QString &word) {
    speller_->addToSession(word);
    ignoredWords_.insert(word);
}

void SonnetSpellChecker::addToPersonalDictionary(const QString &word) {
    speller_->addToPersonal(word);
    ignoredWords_.insert(word);
}

QStringList SonnetSpellChecker::suggestions(const QString &word) {
    if (!speller_->isValid()) {
        return {};
    }
    return speller_->suggest(word);
}

} // namespace Qutepart

#include "hl/loader.h"
#include "hl/syntax_highlighter.h"
#include "qutepart.h"

#include "hl_factory.h"

namespace Qutepart {

class Theme;

QSyntaxHighlighter *makeHighlighter(QObject *parent, const QString &languageId,
                                    const Theme *theme) {
    QSharedPointer<Language> language = loadLanguage(languageId, theme);
    if (!language.isNull()) {
        return new SyntaxHighlighter(parent, language);
    }

    return nullptr;
}

QSyntaxHighlighter *makeHighlighter(QTextDocument *parent, const QString &languageId,
                                    const Theme *theme) {
    QSharedPointer<Language> language = loadLanguage(languageId, theme);
    if (!language.isNull()) {
        return new SyntaxHighlighter(parent, language);
    }

    return nullptr;
}

} // namespace Qutepart

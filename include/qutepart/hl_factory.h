#pragma once

#include <QSyntaxHighlighter>

namespace Qutepart {

/**
 * Choose and load a highlighter.
 *
 * Set as much parameters at posiible to detect language correctly
 *
 * See QSyntaxHighlighter::QSyntaxHighlighter(..) documentation.
 */
QSyntaxHighlighter *makeHighlighter(QObject *parent, const QString &languageId);

QSyntaxHighlighter *makeHighlighter(QTextDocument *parent, const QString &langugeId);

} // namespace Qutepart

#pragma once

#include <QSyntaxHighlighter>
#include <QTextDocument>

#include "language.h"
#include "text_block_user_data.h"

namespace Qutepart {

class SyntaxHighlighter : public QSyntaxHighlighter {
    Q_OBJECT

  public:
    SyntaxHighlighter(QObject *parent, QSharedPointer<Language> language);
    SyntaxHighlighter(QTextDocument *parent, QSharedPointer<Language> language);

  protected:
    void highlightBlock(const QString &text) override;
    QSharedPointer<Language> language;
};

} // namespace Qutepart

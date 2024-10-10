#include <QDebug>

#include "context_switcher.h"
#include "text_block_user_data.h"
#include "text_to_match.h"

#include "language.h"

namespace Qutepart {

Language::Language(const QString &name, const QStringList &extensions, const QStringList &mimetypes,
                   int priority, bool hidden, const QString &indenter,
                   const QSet<QString> &allLanguageKeywords, const QList<ContextPtr> &contexts)
    : name(name), extensions(extensions), mimetypes(mimetypes), priority(priority), hidden(hidden),
      indenter(indenter), allLanguageKeywords_(allLanguageKeywords), contexts(contexts),
      defaultContextStack(contexts[0].data()) {}

void Language::printDescription(QTextStream &out) const {
    out << "Language " << name << "\n";
    out << "\textensions: " << extensions.join(", ") << "\n";
    if (!mimetypes.isEmpty()) {
        out << "\tmimetypes: " << mimetypes.join(", ") << "\n";
    }
    if (priority != 0) {
        out << "\tpriority: " << priority << "\n";
    }
    if (hidden) {
        out << "\thidden";
    }
    if (!indenter.isNull()) {
        out << "\tindenter: " << indenter << "\n";
    }

    foreach (ContextPtr ctx, this->contexts) {
        ctx->printDescription(out);
    }
}

int Language::highlightBlock(QTextBlock block, QVector<QTextLayout::FormatRange> &formats) {
    // qDebug() << "Highlighting: " << block.text();
    ContextStack contextStack = getContextStack(block);

    TextToMatch textToMatch(block.text(), contextStack.currentData());

    QString textTypeMap(textToMatch.text.length(), ' ');

    bool lineContinue = false;

    do {
        // qDebug() << "\tIn context " << contextStack.currentContext()->name();

        const Context *context = contextStack.currentContext();

        contextStack =
            context->parseBlock(contextStack, textToMatch, formats, textTypeMap, lineContinue);
    } while (!textToMatch.isEmpty());

    if (!lineContinue) {
        contextStack = switchAtEndOfLine(contextStack);
    }

    block.setUserData(new TextBlockUserData(textTypeMap, contextStack));

    // Assume that last 32 bits of context address can be used as context ID
    return *((int *)contextStack.currentContext());
}

ContextPtr Language::getContext(const QString &contextName) const {
    foreach (ContextPtr ctx, contexts) {
        if (ctx->name() == contextName) {
            return ctx;
        }
    }

    return ContextPtr();
}

QSet<QString> Language::allLanguageKeywords() const { return allLanguageKeywords_; }

ContextStack Language::getContextStack(QTextBlock block) {
    TextBlockUserData *data = nullptr;

    QTextBlock prevBlock = block.previous();
    if (prevBlock.isValid()) {
        QTextBlockUserData *qtData = prevBlock.userData();
        if (qtData != nullptr) {
            data = dynamic_cast<TextBlockUserData *>(qtData);
        }
    }

    if (data != nullptr) {
        return data->contexts();
    } else {
        return defaultContextStack;
    }
}

ContextStack Language::switchAtEndOfLine(ContextStack contextStack) {
    while (!contextStack.currentContext()->lineEndContext().isNull()) {
        ContextStack oldStack = contextStack;
        contextStack = contextStack.switchContext(contextStack.currentContext()->lineEndContext());
        if (oldStack == contextStack) { // avoid infinite while loop if nothing to switch
            break;
        }
    }

    // this code is not tested, because lineBeginContext is not defined by any
    // xml file
    if (!contextStack.currentContext()->lineBeginContext().isNull()) {
        contextStack =
            contextStack.switchContext(contextStack.currentContext()->lineBeginContext());
    }

    return contextStack;
}

} // namespace Qutepart

#pragma once

#include <QList>
#include <QSet>
#include <QTextBlock>
#include <QTextStream>

#include "context.h"
#include "context_stack.h"

namespace Qutepart {

class Theme;

class Language {
  public:
        Language(const QString &name, const QStringList &extensions, const QStringList &mimetypes,
             int priority, bool hidden, const QString &indenter,
             QString startMultilineComment,
             QString endMultilineComment,
             QString singleLineComment,
             const QSet<QString> &allLanguageKeywords, const QList<ContextPtr> &contexts);

    void printDescription(QTextStream &out) const;

    /* Highlight block and return a state.
    State is a pointer to current context. It can be used to check
    if state changed between runs but not to extract any other data
    */
    int highlightBlock(QTextBlock block, QVector<QTextLayout::FormatRange> &formats);

    inline ContextPtr defaultContext() const { return contexts.first(); }
    ContextPtr getContext(const QString &contextName) const;
    void setTheme(const Theme* theme);

    inline QSet<QString> allLanguageKeywords() const { return allLanguageKeywords_; }
    inline QString getStartMultilineComment() const { return startMultilineComment; }
    inline QString getEndMultilineComment() const { return endMultilineComment; }
    inline QString getSingleLineComment() const { return singleLineComment; }

  protected:
    QString name;
    QString startMultilineComment;
    QString endMultilineComment;
    QString singleLineComment;

    QStringList extensions;
    QStringList mimetypes;
    int priority;
    bool hidden;
    QString indenter;
    QSet<QString> allLanguageKeywords_;

    QList<ContextPtr> contexts;
    ContextStack defaultContextStack;

    ContextStack getContextStack(QTextBlock block);
    ContextStack switchAtEndOfLine(ContextStack contextStack);
};

} // namespace Qutepart

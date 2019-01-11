#pragma once

#include <QTextStream>
#include <QSharedPointer>
#include <QHash>
#include <QTextLayout>

#include "style.h"
#include "context_stack.h"
#include "context_switcher.h"


class Context;
typedef QSharedPointer<Context> ContextPtr;

class AbstractRule;
typedef QSharedPointer<AbstractRule> RulePtr;

class TextToMatch;


class Context {
public:
    Context(const QString& name,
            const QString& attribute,
            const ContextSwitcher& lineEndContext,
            const ContextSwitcher& lineBeginContext,
            const ContextSwitcher& fallthroughContext,
            bool dynamic,
            const QList<RulePtr>& rules);

    void printDescription(QTextStream& out) const;

    QString name() const;

    void resolveContextReferences(const QHash<QString, ContextPtr>& contexts, QString& error);
    void setKeywordParams(const QHash<QString, QStringList>& lists,
                          const QString& deliminators,
                          bool caseSensitive,
                          QString& error);
    void setStyles(const QHash<QString, Style>& styles, QString& error);

    bool dynamic() const {return _dynamic;};
    ContextSwitcher lineBeginContext() const {return _lineBeginContext;};
    ContextSwitcher lineEndContext() const {return _lineEndContext;};

    ContextSwitcher* parseBlock(TextToMatch& textToMatch,
                                QVector<QTextLayout::FormatRange>& formats,
                                QString& textTypeMap,
                                bool& lineContinue) const;

protected:
    void applyMatchResult(TextToMatch& textToMatch, MatchResult& matchRes,
                          QVector<QTextLayout::FormatRange>& formats,
                          QString& textTypeMap);
    QString _name;
    QString attribute;
    ContextSwitcher _lineEndContext;
    ContextSwitcher _lineBeginContext;
    ContextSwitcher fallthroughContext;
    bool _dynamic;
    QList<RulePtr> rules;

    Style style;
};

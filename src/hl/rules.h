/*
 * Copyright (C) 2018-2023 Andrei Kopats
 * Copyright (C) 2023-...  Diego Iastrubni <diegoiast@gmail.com>
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <QHash>
#include <QRegularExpression>
#include <QSharedPointer>
#include <QString>
#include <QTextStream>

#include "context.h"
#include "text_to_match.h"

namespace Qutepart {

class Context;
typedef QSharedPointer<Context> ContextPtr;
class TextToMatch;
class Language;

struct AbstractRuleParams {
    QString attribute; // may be null
    ContextSwitcher context;
    bool lookAhead;
    bool firstNonSpace;
    int column; // -1 if not set
    bool dynamic;
    QString beginRegion;
    QString endRegion;
};

class AbstractRule {
  public:
    AbstractRule(const AbstractRuleParams &params);
    virtual ~AbstractRule() {}

    virtual void printDescription(QTextStream &out) const;
    virtual QString description() const;

    virtual void resolveContextReferences(const QHash<QString, ContextPtr> &contexts,
                                          QString &error);
    virtual void setKeywordParams(const QHash<QString, QStringList> &, bool, const QString &,
                                  QString &) {}
    void setStyles(const QHash<QString, Style> &styles, QString &error);
    void setTheme(const Theme *theme);

    bool lookAhead;
    QSharedPointer<Language> language;

    /* Matching entrypoint. Checks common params and calls tryMatchImpl()
     * Returns true and fills result on a match; result is untouched otherwise.
     */
    bool tryMatch(const TextToMatch &textToMatch, MatchResult &result) const;

  protected:
    friend class Context;
    virtual QString name() const { return "AbstractRule"; }
    virtual QString args() const { return QString(); }

    bool makeMatchResult(MatchResult &result, int length, bool lineContinue = false,
                         const QStringList &data = QStringList()) const;

    /* Rule matching implementation
     * Returns true and fills result on a match; result is untouched otherwise.
     */
    virtual bool tryMatchImpl(const TextToMatch &textToMatch, MatchResult &result) const = 0;

    QString attribute; // may be null
    ContextSwitcher contextSwitcher;
    bool firstNonSpace;
    int column; // -1 if not set
    bool dynamic;
    Style style;
    QString beginRegion;
    QString endRegion;
};

// A rule which has 1 string as a parameter
class AbstractStringRule : public AbstractRule {
  public:
    AbstractStringRule(const AbstractRuleParams &params, const QString &value, bool insensitive);

  protected:
    QString args() const override;
    QString value;
    bool insensitive;
};

class KeywordRule : public AbstractRule {
  public:
    KeywordRule(const AbstractRuleParams &params, const QString &listName);

    void setKeywordParams(const QHash<QString, QStringList> &lists, bool caseSensitive,
                          const QString &, QString &error) override;

    QString name() const override { return "Keyword"; }
    QString args() const override { return listName; }

  private:
    virtual bool tryMatchImpl(const TextToMatch &textToMatch, MatchResult &result) const override;

    QString listName;
    QHash<QString, bool> items;
    bool caseSensitive;
    DeliminatorSet deliminators;
};

class DetectCharRule : public AbstractRule {
  public:
    DetectCharRule(const AbstractRuleParams &params, QChar value, int index);

    QString name() const override { return "DetectChar"; }
    QString args() const override;

  private:
    virtual bool tryMatchImpl(const TextToMatch &textToMatch, MatchResult &result) const override;

    QChar value;
    int index;
};

class Detect2CharsRule : public AbstractStringRule {
    using AbstractStringRule::AbstractStringRule;

  public:
    QString name() const override { return "Detect2Chars"; }

  private:
    bool tryMatchImpl(const TextToMatch &textToMatch, MatchResult &result) const override;
};

class AnyCharRule : public AbstractStringRule {
    using AbstractStringRule::AbstractStringRule;

  public:
    QString name() const override { return "AnyChar"; }

  private:
    virtual bool tryMatchImpl(const TextToMatch &textToMatch, MatchResult &result) const override;
};

class StringDetectRule : public AbstractStringRule {
    using AbstractStringRule::AbstractStringRule;

  public:
    QString name() const override { return "StringDetect"; }

  private:
    virtual bool tryMatchImpl(const TextToMatch &textToMatch, MatchResult &result) const override;
};

class WordDetectRule : public AbstractStringRule {
    using AbstractStringRule::AbstractStringRule;

  public:
    QString name() const override { return "WordDetect"; }
    void setKeywordParams(const QHash<QString, QStringList> &lists, bool caseSensitive,
                          const QString &, QString &error) override;

  private:
    virtual bool tryMatchImpl(const TextToMatch &textToMatch, MatchResult &result) const override;
    DeliminatorSet mDeliminatorSet;
};

class RegExpRule : public AbstractRule {
  public:
    RegExpRule(const AbstractRuleParams &params, const QString &value, bool insensitive,
               bool minimal, bool wordStart, bool lineStart);

  private:
    QString name() const override { return "RegExpr"; }
    QString args() const override;

    QRegularExpression compileRegExp(const QString &pattern) const;
    virtual bool tryMatchImpl(const TextToMatch &textToMatch, MatchResult &result) const override;

    QString value;
    bool insensitive;
    bool minimal;
    bool wordStart;
    bool lineStart;
    QRegularExpression regExp;
};

class AbstractNumberRule : public AbstractRule {
  public:
    AbstractNumberRule(const AbstractRuleParams &params, const QList<RulePtr> &childRules);

    void printDescription(QTextStream &out) const override;

  protected:
    bool tryMatchImpl(const TextToMatch &textToMatch, MatchResult &result) const override;
    virtual int tryMatchText(const QStringView &text) const = 0;
    int countDigits(const QStringView &text) const;

    QList<RulePtr> childRules;
};

class IntRule : public AbstractNumberRule {
    using AbstractNumberRule::AbstractNumberRule;

  public:
    QString name() const override { return "Int"; }

  private:
    virtual int tryMatchText(const QStringView &text) const override;
};

class FloatRule : public AbstractNumberRule {
    using AbstractNumberRule::AbstractNumberRule;

  public:
    QString name() const override { return "Float"; }

  private:
    int tryMatchText(const QStringView &text) const override;
};

class HlCOctRule : public AbstractRule {
    using AbstractRule::AbstractRule;

  public:
    QString name() const override { return "HlCOct"; }

  private:
    bool tryMatchImpl(const TextToMatch &textToMatch, MatchResult &result) const override;
};

class HlCHexRule : public AbstractRule {
    using AbstractRule::AbstractRule;

  public:
    QString name() const override { return "HlCHex"; }

  private:
    bool tryMatchImpl(const TextToMatch &textToMatch, MatchResult &result) const override;
};

class HlCStringCharRule : public AbstractRule {
    using AbstractRule::AbstractRule;

  public:
    QString name() const override { return "HlCStringChar"; }

  private:
    bool tryMatchImpl(const TextToMatch &textToMatch, MatchResult &result) const override;
};

class HlCCharRule : public AbstractRule {
    using AbstractRule::AbstractRule;

  public:
    QString name() const override { return "HlCChar"; }

  private:
    bool tryMatchImpl(const TextToMatch &textToMatch, MatchResult &result) const override;
};

class RangeDetectRule : public AbstractRule {
  public:
    RangeDetectRule(const AbstractRuleParams &params, const QString &char0, const QString &char1);
    QString name() const override { return "RangeDetect"; }
    QString args() const override;

  private:
    bool tryMatchImpl(const TextToMatch &textToMatch, MatchResult &result) const override;

    const QString char0;
    const QString char1;
};

class LineContinueRule : public AbstractRule {
    using AbstractRule::AbstractRule;

  public:
    QString name() const override { return "LineContinue"; }

  private:
    bool tryMatchImpl(const TextToMatch &textToMatch, MatchResult &result) const override;
};

class IncludeRulesRule : public AbstractRule {
  public:
    IncludeRulesRule(const AbstractRuleParams &params, const QString &contextName);

    QString name() const override { return "IncludeRules"; }
    QString args() const override { return contextName; }

    void resolveContextReferences(const QHash<QString, ContextPtr> &contexts,
                                  QString &error) override;

  private:
    bool tryMatchImpl(const TextToMatch &textToMatch, MatchResult &result) const override;

    QString contextName;
    ContextPtr context;
};

class DetectSpacesRule : public AbstractRule {
    using AbstractRule::AbstractRule;

  public:
    QString name() const override { return "DetectSpaces"; }

  private:
    bool tryMatchImpl(const TextToMatch &textToMatch, MatchResult &result) const override;
};

class DetectIdentifierRule : public AbstractRule {
    using AbstractRule::AbstractRule;

  public:
    QString name() const override { return "DetectIdentifier"; }

  private:
    bool tryMatchImpl(const TextToMatch &textToMatch, MatchResult &result) const override;
};

} // namespace Qutepart

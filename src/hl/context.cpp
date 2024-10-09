#include <QDebug>
#include <QScopedPointer>

#include "context.h"
#include "match_result.h"
#include "rules.h"
#include "text_to_match.h"
#include "theme.h"

namespace Qutepart {

Context::Context(const QString &name, const QString &attribute,
                 const ContextSwitcher &lineEndContext, const ContextSwitcher &lineBeginContext,
                 const ContextSwitcher &lineEmptyContext, const ContextSwitcher &fallthroughContext,
                 bool dynamic, const QList<RulePtr> &rules)
    : _name(name), attribute(attribute), _lineEndContext(lineEndContext),
      _lineBeginContext(lineBeginContext), _lineEmptyContext(lineEmptyContext),
      fallthroughContext(fallthroughContext), _dynamic(dynamic), rules(rules) {}

void Context::printDescription(QTextStream &out) const {
    out << "\tContext " << this->_name << "\n";
    out << "\t\tattribute: " << attribute << "\n";
    if (!_lineEndContext.isNull()) {
        out << "\t\tlineEndContext: " << _lineEndContext.toString() << "\n";
    }
    if (!_lineBeginContext.isNull()) {
        out << "\t\tlineBeginContext: " << _lineBeginContext.toString() << "\n";
    }
    if (!_lineEmptyContext.isNull()) {
        out << "\t\tlineEmptyContext: " << _lineEmptyContext.toString() << "\n";
    }
    if (!fallthroughContext.isNull()) {
        out << "\t\tfallthroughContext: " << fallthroughContext.toString() << "\n";
    }
    if (_dynamic) {
        out << "\t\tdynamic\n";
    }

    foreach (RulePtr rule, rules) {
        rule->printDescription(out);
    }
}

QString Context::name() const { return _name; }

void Context::resolveContextReferences(const QHash<QString, ContextPtr> &contexts, QString &error,
                                       const Theme *theme) {
    _lineEndContext.resolveContextReferences(contexts, error, theme);
    if (!error.isNull()) {
        return;
    }

    _lineBeginContext.resolveContextReferences(contexts, error, theme);
    if (!error.isNull()) {
        return;
    }

    _lineEmptyContext.resolveContextReferences(contexts, error, theme);
    if (!error.isNull()) {
        return;
    }

    fallthroughContext.resolveContextReferences(contexts, error, theme);
    if (!error.isNull()) {
        return;
    }

    foreach (RulePtr rule, rules) {
        rule->resolveContextReferences(contexts, error, theme);
        if (!error.isNull()) {
            return;
        }
    }
}

void Context::setKeywordParams(const QHash<QString, QStringList> &lists,
                               const QString &deliminatorSet, bool caseSensitive, QString &error) {
    foreach (RulePtr rule, rules) {
        rule->setKeywordParams(lists, caseSensitive, deliminatorSet, error);
        if (!error.isNull()) {
            break;
        }
    }
}

void Context::setStyles(const QHash<QString, Style> &styles, QString &error) {
    if (!attribute.isNull()) {
        if (!styles.contains(attribute)) {
            error = QString("Not found context '%1' attribute '%2'").arg(_name, attribute);
            return;
        }
        style = styles[attribute];
        style.updateTextType(attribute);
    }

    foreach (RulePtr rule, rules) {
        rule->setStyles(styles, error);
        if (!error.isNull()) {
            break;
        }
    }
}

void Context::setTheme(const Theme *theme) {
    if (theme == this->theme) {
        return;
    }
    this->theme = theme;
#if 0
    auto ts = theme->textStyles;
    auto n = _name;

    qDebug() << "Context " << n << " setting theme";
    if (theme) {
        if (theme->textStyles.contains(n)) {
            QStringHash styleProperties = theme->textStyles[n];
            applyProperties(style.format(), styleProperties, n);
            // } else {
            // qDebug() << "** Context" << n << "not in theme";
        }

        // TODO: get customs styles for language
        // if (theme->customStyles.contains(langName)) {
        //     auto custom = theme.customStyles[langName];
        //     // if (custom.contains(contextName))
        //     {
        //         auto styleProperties = custom[contextName];
        //         applyProperties(format, styleProperties);
        //     }
        // }

        for (auto rule : rules) {
            // auto ts = theme->textStyles;
            auto n = rule->style.name;
            if (theme->textStyles.contains(n)) {
                QStringHash styleProperties = theme->textStyles[n];
                applyProperties(rule->style.format(), styleProperties, n);
                // } else {
                // qDebug() << "** Context" << n << "not in theme";
            }

            if (rule->context.context()) {
                rule->context.context()->setTheme(theme);
            }
        }
    }

    if (_lineEndContext.context()) {
        _lineEndContext.context()->setTheme(theme);
    }
    if (_lineBeginContext.context()) {
        _lineBeginContext.context()->setTheme(theme);
    }
    if (_lineEmptyContext.context()) {
        _lineEmptyContext.context()->setTheme(theme);
    }
    if (fallthroughContext.context()) {
        fallthroughContext.context()->setTheme(theme);
    }
#endif
}

void appendFormat(QVector<QTextLayout::FormatRange> &formats, int start, int length,
                  const QTextCharFormat &format) {

    if ((!formats.isEmpty()) && (formats.last().start + formats.last().length) == start &&
        formats.last().format == format) {
        formats.last().length += length;
    } else {
        QTextLayout::FormatRange fmtRange;
        fmtRange.start = start;
        fmtRange.length = length;
        fmtRange.format = format;
        formats.append(fmtRange);
    }
}

void fillTextTypeMap(QString &textTypeMap, int start, int length, QChar textType) {
    for (int i = start; i < start + length; i++) {
        textTypeMap[i] = textType;
    }
}

// Helper function for parseBlock()
void Context::applyMatchResult(const TextToMatch &textToMatch, const MatchResult *matchRes,
                               const Context *context, QVector<QTextLayout::FormatRange> &formats,
                               QString &textTypeMap) const {
    QSharedPointer<QTextCharFormat> format = matchRes->style.format();
    if (format.isNull()) {
        format = context->style.format();
    }

    if (!format.isNull()) {
        appendFormat(formats, textToMatch.currentColumnIndex, matchRes->length, *format);

#if 1
        // TODO - remove debug code
        // Extract the text that was matched
        auto matchedText = textToMatch.text.mid(textToMatch.currentColumnIndex, matchRes->length);

        // Get the color of the format
        QColor color = format->foreground().color();

        // // Print debug information
        auto n = context->name();
        n = matchRes->style.name;
        qDebug() << QString("context: %1 (0x%2), color %3 - %4")
                        .arg(n)
                        .arg(reinterpret_cast<quintptr>(context), 0, 16)
                        .arg(color.name(QColor::HexRgb))
                        .arg(matchedText);
#endif
    }

    QChar textType = matchRes->style.textType();
    if (textType == 0) {
        textType = context->style.textType();
    }
    fillTextTypeMap(textTypeMap, textToMatch.currentColumnIndex, matchRes->length, textType);
}

// Parse block. Exits, when reached end of the text, or when context is switched
const ContextStack Context::parseBlock(const ContextStack &contextStack, TextToMatch &textToMatch,
                                       QVector<QTextLayout::FormatRange> &formats,
                                       QString &textTypeMap, bool &lineContinue) const {
    textToMatch.contextData = &contextStack.currentData();

    if (textToMatch.isEmpty() && (!_lineEmptyContext.isNull())) {
        return contextStack.switchContext(_lineEmptyContext);
    }

    while (!textToMatch.isEmpty()) {
        QScopedPointer<MatchResult> matchRes(tryMatch(textToMatch));

        if (!matchRes.isNull()) {
            lineContinue = matchRes->lineContinue;

            if (matchRes->nextContext.isNull()) {
                applyMatchResult(textToMatch, matchRes.data(), this, formats, textTypeMap);
                textToMatch.shift(matchRes->length);
            } else {
                ContextStack newContextStack =
                    contextStack.switchContext(matchRes->nextContext, matchRes->data);

                applyMatchResult(textToMatch, matchRes.data(), newContextStack.currentContext(),
                                 formats, textTypeMap);
                textToMatch.shift(matchRes->length);

                return newContextStack;
            }
        } else {
            lineContinue = false;
            if (!this->style.format().isNull()) {
                appendFormat(formats, textToMatch.currentColumnIndex, 1, *(this->style.format()));
            }

            textTypeMap[textToMatch.currentColumnIndex] = this->style.textType();

            if (!this->fallthroughContext.isNull()) {
                return contextStack.switchContext(this->fallthroughContext);
            }

            textToMatch.shiftOnce();
        }
    }

    return contextStack;
}

MatchResult *Context::tryMatch(const TextToMatch &textToMatch) const {
    foreach (RulePtr rule, rules) {
        MatchResult *matchRes = rule->tryMatch(textToMatch);
        if (matchRes != nullptr) {
            return matchRes;
        }
    }

    return nullptr;
}

} // namespace Qutepart

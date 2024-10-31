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

void Context::resolveContextReferences(const QHash<QString, ContextPtr> &contexts, QString &error) {
    _lineEndContext.resolveContextReferences(contexts, error);
    if (!error.isNull()) {
        return;
    }

    _lineBeginContext.resolveContextReferences(contexts, error);
    if (!error.isNull()) {
        return;
    }

    _lineEmptyContext.resolveContextReferences(contexts, error);
    if (!error.isNull()) {
        return;
    }

    fallthroughContext.resolveContextReferences(contexts, error);
    if (!error.isNull()) {
        return;
    }

    foreach (RulePtr rule, rules) {
        rule->resolveContextReferences(contexts, error);
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

auto applyProperties(QTextCharFormat *format, QStringHash &styleProperties) -> void {
    for (auto it = styleProperties.constBegin(); it != styleProperties.constEnd(); ++it) {
        auto &key = it.key();
        auto &value = it.value();

        if (key == "text-color") {
            auto val = QColor(value);
            format->setForeground(val);
        } else if (key == "selected-text-color") {
            // TODO
        } else if (key == "background-color") {
            auto val = QColor(value);
            format->setBackground(val);
        } else if (key == "bold") {
            auto val = value.toLower() == "true";
            format->setFontWeight(val ? QFont::Bold : QFont::Normal);
        } else if (key == "italic") {
            auto val = value.toLower() == "true";
            format->setFontItalic(val);
        } else if (key == "underline") {
            auto val = value.toLower() == "true";
            format->setFontUnderline(val);
        } else if (key == "strike-through") {
            auto val = value.toLower() == "true";
            format->setFontStrikeOut(val);
        } else if (key == "font-family") {
            format->setFontFamilies({value});
        } else if (key == "font-size") {
            bool ok;
            int fontSize = value.toInt(&ok);
            if (ok) {
                format->setFontPointSize(fontSize);
            }
        }
        // Add more properties as needed
    }
}


// Helper function for parseBlock()
void Context::applyMatchResult(const TextToMatch &textToMatch, const MatchResult *matchRes,
                               const Context *context, QVector<QTextLayout::FormatRange> &formats,
                               QString &textTypeMap, const Theme* theme) const {
    QSharedPointer<QTextCharFormat> format = matchRes->style.format();
    QTextCharFormat displayFormat;

    if (format.isNull()) {
        displayFormat = *context->style.format().get();
    } else {
        // find the theme's format
        displayFormat = *format.get();
        if (theme) {
            auto fixedName = style.getDefStyle().mid(2).toString();
            // qDebug() << "Patching (context)" << fixedName << "color:" << displayFormat.foreground().color().name(QColor::HexRgb) << " -> " << theme->textStyles[fixedName].value("text-color");
            if (theme->textStyles.contains(fixedName)) {
                auto styleProperties = theme->textStyles[fixedName];
                applyProperties(&displayFormat, styleProperties);
            }
        }
    }

    if (!format.isNull()) {
        appendFormat(formats, textToMatch.currentColumnIndex, matchRes->length, *format);
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
                                       QString &textTypeMap, bool &lineContinue, const Theme *theme) const {
    textToMatch.contextData = &contextStack.currentData();

    if (textToMatch.isEmpty() && (!_lineEmptyContext.isNull())) {
        return contextStack.switchContext(_lineEmptyContext);
    }

    Style displayStyle = this->style;
    if (theme) {
        auto fixedName = style.getDefStyle().mid(2).toString();
        // qDebug() << "Patching (style)" << fixedName;
        if (theme->textStyles.contains(fixedName)) {
            auto styleProperties = theme->textStyles[fixedName];
            applyProperties(displayStyle.format().get(), styleProperties);
        }
    }

    while (!textToMatch.isEmpty()) {
        QScopedPointer<MatchResult> matchRes(tryMatch(textToMatch));

        if (!matchRes.isNull()) {
            lineContinue = matchRes->lineContinue;

            if (matchRes->nextContext.isNull()) {
                applyMatchResult(textToMatch, matchRes.data(), this, formats, textTypeMap, theme);
                textToMatch.shift(matchRes->length);
            } else {
                ContextStack newContextStack =
                    contextStack.switchContext(matchRes->nextContext, matchRes->data);

                applyMatchResult(textToMatch, matchRes.data(), newContextStack.currentContext(),
                                 formats, textTypeMap, theme);
                textToMatch.shift(matchRes->length);

                return newContextStack;
            }
        } else {
            lineContinue = false;
            if (!displayStyle.format().isNull()) {
                appendFormat(formats, textToMatch.currentColumnIndex, 1, *(displayStyle.format()));
            }

            textTypeMap[textToMatch.currentColumnIndex] = displayStyle.textType();

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


#include <QColor>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMap>
#include <QString>
#include <QTextFormat>
#include <QVector>
#include <qutepart.h>

#include <hl/rules.h>
#include <hl/syntax_highlighter.h>

#include "bracket_highlighter.h"
#include "hl/loader.h"
#include "theme.h"

namespace Qutepart {

void applyStyleToFormat(QTextCharFormat &format, const QStringHash &styleProperties) {
    for (auto it = styleProperties.constBegin(); it != styleProperties.constEnd(); ++it) {
        const QString &key = it.key();
        const QString &value = it.value();

        if (key == "text-color" || key == "selected-text-color") {
            format.setForeground(QColor(value));
        } else if (key == "background-color") {
            format.setBackground(QColor(value));
        } else if (key == "bold") {
            auto val = value.toLower() == "true";
            format.setFontWeight(val ? QFont::Bold : QFont::Normal);
        } else if (key == "italic") {
            auto val = value.toLower() == "true";
            format.setFontItalic(val);
        } else if (key == "underline") {
            auto val = value.toLower() == "true";
            format.setFontUnderline(val);
        } else if (key == "strike-through") {
            auto val = value.toLower() == "true";
            format.setFontStrikeOut(val);
        } else if (key == "font-family") {
            format.setFontFamilies({value});
        } else if (key == "font-size") {
            bool ok;
            int fontSize = value.toInt(&ok);
            if (ok) {
                format.setFontPointSize(fontSize);
            }
        }
        // Add more properties as needed
    }
}

Theme::Theme() = default;

auto Theme::loadTheme(const QString &filename) -> bool {
    auto file = QFile(filename);
    if (!file.open(QIODevice::ReadOnly)) {
        return false;
    }

    auto jsonData = file.readAll();
    auto document = QJsonDocument::fromJson(jsonData);

    if (document.isNull() || !document.isObject()) {
        return false;
    }

    auto themeData = document.object();

    // Parse custom styles
    QJsonObject customStylesObj = themeData["custom-styles"].toObject();
    for (auto categoryIt = customStylesObj.begin(); categoryIt != customStylesObj.end();
         ++categoryIt) {
        QString category = categoryIt.key();
        QJsonObject categoryObj = categoryIt.value().toObject();

        QHash<QString, QStringHash> categoryStyles;
        for (auto styleIt = categoryObj.begin(); styleIt != categoryObj.end(); ++styleIt) {
            QString styleName = styleIt.key();
            QJsonObject styleObj = styleIt.value().toObject();

            QStringHash styleProperties;
            for (auto propIt = styleObj.begin(); propIt != styleObj.end(); ++propIt) {
                styleProperties[propIt.key()] = propIt.value().toString();
            }

            categoryStyles[styleName] = styleProperties;
        }

        customStyles[category] = categoryStyles;
    }

    // Parse editor colors
    auto editorColorsObj = themeData["editor-colors"].toObject();
    for (auto it = editorColorsObj.begin(); it != editorColorsObj.end(); ++it) {
        auto colorName = it.key();
        auto color = QColor(it.value().toString());
        editorColors[colorName] = color;
    }

    // Parse text styles
    QJsonObject textStylesObj = themeData["text-styles"].toObject();
    for (auto it = textStylesObj.begin(); it != textStylesObj.end(); ++it) {
        QString styleName = it.key();
        QJsonObject styleObj = it.value().toObject();

        QStringHash styleProperties;
        for (auto propIt = styleObj.begin(); propIt != styleObj.end(); ++propIt) {
            styleProperties[propIt.key()] = propIt.value().toString();
        }

        textStyles[styleName] = styleProperties;
    }

    // Parse metadata
    auto metaDataObj = themeData["metadata"].toObject();
    auto copyrightArray = metaDataObj["copyright"].toArray();
    for (const auto &copyright : copyrightArray) {
        metaData.copyright.push_back(copyright.toString());
    }
    metaData.license = metaDataObj["license"].toString();
    metaData.name = metaDataObj["name"].toString();
    metaData.revision = metaDataObj["revision"].toInt();

    return true;
}

auto Theme::getEditorColors() const -> const QHash<QString, QColor> & { return editorColors; }

auto Theme::getMetaData() const -> const ThemeMetaData & { return metaData; }

#if 0
auto Theme::applyTo(Qutepart *editor) -> void {

    // CodeFolding
    // CurrentLineNumber
    // IconBorder
    // LineNumbers
    // MarkBookmark
    // MarkBreakpointActive
    // MarkBreakpointDisabled
    // MarkBreakpointReached
    // MarkError
    // MarkExecution
    // MarkWarning
    // ModifiedLines
    // ReplaceHighlight
    // SavedLines
    // SearchHighlight
    // Separator
    // SpellChecking
    // TabMarker
    // TemplateBackground
    // TemplateFocusedPlaceholder
    // TemplatePlaceholder
    // Placeholder
    // TextSelection

    // editor->lineNumberColor = this->editorColors[Theme::Colors::LineNumbers];
    editor->lineNumberColor = this->editorColors[Theme::Colors::LineNumbers];
    editor->currentLineNumberColor = this->editorColors[Theme::Colors::CurrentLineNumber];
    editor->iconBorderColor = this->editorColors[Theme::Colors::IconBorder];

    editor->lineLengthEdgeColor_ = this->editorColors[Theme::Colors::WordWrapMarker];
    editor->currentLineColor_ = this->editorColors[Theme::Colors::CurrentLine];
    editor->indentColor_ = this->editorColors[Theme::Colors::IndentationLine];
    editor->bracketHighlighter_->matchedColor = this->editorColors[Theme::Colors::BracketMatching];
    editor->bracketHighlighter_->nonMatchedColor =
        this->editorColors[Theme::Colors::BracketMatching];

    if (editorColors.contains(Theme::Colors::BackgroundColor) &&
        editorColors[Theme::Colors::BackgroundColor].isValid()) {
        QPalette p(editor->palette());
        p.setColor(QPalette::Base, editorColors[Colors::BackgroundColor]);
        // p.setColor(QPalette::Text, editorColors[Theme::]);
        editor->setPalette(p);
    }

    auto syntax = qSharedPointerCast<SyntaxHighlighter>(editor->highlighter_);
    if (!syntax) {
        return;
    }
    QString error;
    QHashIterator<QString, QStringHash> i(textStyles);
    while (i.hasNext()) {
        i.next();
        auto name = i.key();
        auto style = i.value();

        auto context = syntax->language->getContext(name);
        if (context) {
            applyStyleToFormat(*context->style._format.get(), style);
        } else {
            auto ctx = loadExternalContext("##" + name);
            if (ctx) {
                applyStyleToFormat(*ctx->style._format.get(), style);
            }
            qDebug() << "Could not patch context " << name;
        }
    }

    auto id = syntax->language->name;
    if (customStyles.contains(id)) {
        auto languageStyles = customStyles[id];
        QHashIterator<QString, QStringHash> i(languageStyles);
        while (i.hasNext()) {
            i.next();
            auto name = i.key();
            auto style = i.value();

            for (auto context : syntax->language->contexts) {
                if (context->name() != name) {
                    continue;
                }
                qDebug() << "Patching format" << name << "for language" << id;
                applyStyleToFormat(*context->style._format.get(), style);
            }
        }
    }
    syntax->rehighlight();
}
#endif

} // namespace Qutepart

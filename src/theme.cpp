
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

} // namespace Qutepart

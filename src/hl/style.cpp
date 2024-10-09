#include "style.h"
#include "theme.h"

namespace Qutepart {

QSharedPointer<QTextCharFormat> defaultFormat(const QString &style, QString &error) {
    QSharedPointer<QTextCharFormat> format(new QTextCharFormat());

    bool bold = false;
    bool underline = false;
    QString colorName;
    QString bgColorName;

    if (style == "dsNormal") {
    } else if (style == "dsKeyword") {
        bold = true;
    } else if (style == "dsFunction") {
        colorName = "#644a9a";
    } else if (style == "dsVariable") {
        colorName = "#0057ad";
    } else if (style == "dsControlFlow") {
        bold = true;
    } else if (style == "dsOperator") {
    }

    else if (style == "dsBuiltIn") {
        colorName = "#644a9a";
        bold = true;
    } else if (style == "dsExtension") {
        colorName = "#0094fe";
        bold = true;
    } else if (style == "dsPreprocessor") {
        colorName = "#006e28";
    } else if (style == "dsAttribute") {
        colorName = "#0057ad";
    }

    else if (style == "dsChar") {
        colorName = "#914c9c";
    } else if (style == "dsSpecialChar") {
        colorName = "#3dade8";
    } else if (style == "dsString") {
        colorName = "#be0303";
    } else if (style == "dsVerbatimString") {
        colorName = "#be0303";
    } else if (style == "dsSpecialString") {
        colorName = "#fe5500";
    } else if (style == "dsImport") {
        colorName = "#b969c3";
    }

    else if (style == "dsDataType") {
        colorName = "#0057ad";
    } else if (style == "dsDecVal") {
        colorName = "#af8000";
    } else if (style == "dsBaseN") {
        colorName = "#af8000";
    } else if (style == "dsFloat") {
        colorName = "#af8000";
    }

    else if (style == "dsConstant") {
        bold = true;
    }

    else if (style == "dsComment") {
        colorName = "#888786";
    } else if (style == "dsDocumentation") {
        colorName = "#608880";
    } else if (style == "dsAnnotation") {
        colorName = "#0094fe";
    } else if (style == "dsCommentVar") {
        colorName = "#c960c9";
    }

    else if (style == "dsRegionMarker") {
        colorName = "#0057ad";
        bgColorName = "#e0e9f8";
    } else if (style == "dsInformation") {
        colorName = "#af8000";
    } else if (style == "dsWarning") {
        colorName = "#be0303";
    } else if (style == "dsAlert") {
        colorName = "#be0303";
        bgColorName = "#f7e6e6";
        bold = true;
    } else if (style == "dsOthers") {
        colorName = "#006e28";
    } else if (style == "dsError") {
        colorName = "#bf0303";
        underline = true;
    } else {
        error = QString("Unknown default style '%1'").arg(style);
        return format;
    }

    if (bold) {
        format->setFontWeight(QFont::Bold);
    }

    if (underline) {
        format->setFontUnderline(true);
    }

    if (!colorName.isNull()) {
        format->setForeground(QColor(colorName));
    }

    if (!bgColorName.isNull()) {
        format->setBackground(QColor(bgColorName));
    }

    return format;
}

QSharedPointer<QTextCharFormat> makeFormat(const QString &defStyle, const QString &color,
                                           const QString & /*selColor*/,
                                           const QHash<QString, bool> &flags, QString &error) {
    QSharedPointer<QTextCharFormat> format = defaultFormat(defStyle, error);
    if (!error.isNull()) {
        return QSharedPointer<QTextCharFormat>();
    }

    if (!color.isNull()) {
        format->setForeground(QColor(color));
    }

    if (flags.contains("italic")) {
        format->setFontItalic(flags["italic"]);
    }

    if (flags.contains("bold")) {
        if (flags["bold"]) {
            format->setFontWeight(QFont::Bold);
        } else {
            format->setFontWeight(QFont::Normal);
        }
    }

    if (flags.contains("underline")) {
        format->setFontUnderline(flags["underline"]);
    }

    if (flags.contains("strikeout")) {
        format->setFontStrikeOut(flags["strikeout"]);
    }

    return format;
}

/* ' ' for code
   'c' for comments
   'b' for block comments
   'h' for here documents
*/
char detectTextType(const QString &attribute, const QString &defStyleName) {
    if (attribute.toLower().contains("here") && defStyleName == "dsOthers") {
        return 'h'; // ruby
    } else if (attribute.toLower().contains("block") && defStyleName == "dsComment") {
        return 'b';
    }

    if (defStyleName == "dsString" || defStyleName == "dsRegionMarker" ||
        defStyleName == "dsChar" || defStyleName == "dsOthers") {
        return 's';
    }

    if (defStyleName == "dsComment") {
        return 'c';
    }

    return ' ';
}

auto applyProperties(QSharedPointer<QTextCharFormat> format, QStringHash &styleProperties,
                     QString _name) -> void {
    Q_UNUSED(_name);

    qDebug() << "** Updating context" << _name << (void *)(format.get()) << styleProperties;
    for (auto it = styleProperties.constBegin(); it != styleProperties.constEnd(); ++it) {
        const QString &key = it.key();
        const QString &value = it.value();

        if (key == "text-color") {
            auto val = QColor(value);
            // val = QColor(0x00ff00);
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
};

Style makeStyle(const QString &name, const QString &defStyleName, const QString &color,
                const QString &selColor, const QHash<QString, bool> &flags, QString &error,
                const Theme *theme) {
    QSharedPointer<QTextCharFormat> format =
        makeFormat(defStyleName, color, selColor, flags, error);
    if (!error.isNull()) {
        return Style();
    }

    if (theme) {
        // TODO
        if (theme->textStyles.contains(name)) {
            QStringHash styleProperties = theme->textStyles[name];
            applyProperties(format, styleProperties, name);
        } else {
            qDebug() << "** Context" << name << "not in theme" << theme->metaData.name;
            if (theme->textStyles.contains(name)) {
                QStringHash styleProperties = theme->textStyles[name];
                applyProperties(format, styleProperties, name);
            }
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
    }

    return Style(name, defStyleName, format);
}

Style::Style() : _textType(' ') {}

Style::Style(const QString &name, const QString &defStyleName,
             QSharedPointer<QTextCharFormat> format)
    : name(name), _format(format), _textType(detectTextType(QString(), defStyleName)),
      defStyleName(defStyleName) {}

void Style::updateTextType(const QString &attribute) {
    _textType = detectTextType(attribute, defStyleName);
}

} // namespace Qutepart

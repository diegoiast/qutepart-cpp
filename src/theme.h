#pragma once

#include <QHash>
#include <QJsonObject>
#include <QString>
#include <hl/style.h>

class QSyntaxHighlighter;
namespace Qutepart {

class Qutepart;

typedef QHash<QString, QString> QStringHash;

struct ThemeMetaData {
    QString copyright;
    QString license;
    QString name;
    int revision;
};

class Theme {
  public:
    struct Colors {
        static constexpr const char *BackgroundColor = "BackgroundColor";
        static constexpr const char *BracketMatching = "BracketMatching";
        static constexpr const char *CodeFolding = "CodeFolding";
        static constexpr const char *CurrentLine = "CurrentLine";
        static constexpr const char *CurrentLineNumber = "CurrentLineNumber";
        static constexpr const char *IconBorder = "IconBorder";
        static constexpr const char *IndentationLine = "IndentationLine";
        static constexpr const char *LineNumbers = "LineNumbers";
        static constexpr const char *MarkBookmark = "MarkBookmark";
        static constexpr const char *MarkBreakpointActive = "MarkBreakpointActive";
        static constexpr const char *MarkBreakpointDisabled = "MarkBreakpointDisabled";
        static constexpr const char *MarkBreakpointReached = "MarkBreakpointReached";
        static constexpr const char *MarkError = "MarkError ";
        static constexpr const char *MarkExecution = "MarkExecution";
        static constexpr const char *MarkWarning = "MarkWarning";
        static constexpr const char *ModifiedLines = "ModifiedLines";
        static constexpr const char *ReplaceHighlight = "ReplaceHighlight";
        static constexpr const char *SavedLines = "SavedLines";
        static constexpr const char *SearchHighlight = "SearchHighlight";
        static constexpr const char *Separator = "Separator";
        static constexpr const char *SpellChecking = "SpellChecking";
        static constexpr const char *TabMarker = "TabMarker";
        static constexpr const char *TemplateBackground = "TemplateBackground";
        static constexpr const char *TemplateFocusedPlaceholder = "TemplateFocusedPlaceholder";
        static constexpr const char *TemplatePlaceholder = "TemplatePlaceholder";
        static constexpr const char *TemplateReadOnlyPlaceholder = "TemplateReadOnlyPlaceholder";
        static constexpr const char *TextSelection = "TextSelection";
        static constexpr const char *WordWrapMarker = "WordWrapMarker";
    };

    Theme();
    auto loadTheme(const QString &filename) -> bool;
    auto getEditorColors() const -> const QHash<QString, QColor> &;
    auto getMetaData() const -> const ThemeMetaData &;

    // private:
    QHash<QString, QHash<QString, QStringHash>> customStyles;
    QHash<QString, QColor> editorColors;
    QHash<QString, QStringHash> textStyles;
    ThemeMetaData metaData;
};

} // namespace Qutepart
#pragma once

#include <QSharedPointer>
#include <QTextCharFormat>

namespace Qutepart {

class Theme;

class Style {
  public:
    Style();
    Style(const QString &defStyleName, const QTextCharFormat &format);

    /* Called by some clients.
       If the style knows attribute it can better detect textType
     */
    void updateTextType(const QString &attribute);

    inline char textType() const { return _textType; }
    inline const QStringView getDefStyle() const { return defStyleName; }
    inline const QTextCharFormat format() const {
        return displayFormat;
    }

    void setTheme(const Theme *theme);
    inline const Theme* getTheme() const { return theme; }

  private:
    QTextCharFormat savedFormat;
    QTextCharFormat displayFormat;
    char _textType;

    QString defStyleName;
    const Theme* theme = nullptr;
};

Style makeStyle(const QString &defStyleName, const QString &color,
                const QString & /*selColor*/, const QHash<QString, bool> &flags, QString &error);

} // namespace Qutepart

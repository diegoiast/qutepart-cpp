#pragma once

#include <QSharedPointer>
#include <QTextCharFormat>

namespace Qutepart {

class Style {
  public:
    Style();
    Style(const QString &defStyleName, QSharedPointer<QTextCharFormat> format);

    /* Called by some clients.
       If the style knows attribute it can better detect textType
     */
    void updateTextType(const QString &attribute);

    inline char textType() const { return _textType; };
    inline const QSharedPointer<QTextCharFormat> format() const { return _format; }

    inline const QStringView getDefStyle() const { return defStyleName; }

  private:
    QSharedPointer<QTextCharFormat> _format;
    char _textType;

    QString defStyleName;
};

Style makeStyle(const QString &defStyleName, const QString &color,
                const QString & /*selColor*/, const QHash<QString, bool> &flags, QString &error);

} // namespace Qutepart

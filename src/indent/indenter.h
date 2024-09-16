#pragma once

#include <memory>

#include <QKeyEvent>
#include <QObject>
#include <QString>
#include <QTextBlock>

#include "alg_impl.h"
#include "qutepart.h"

namespace Qutepart {

class Indenter : public QObject {
  public:
    Indenter();

    void setAlgorithm(IndentAlg alg);

    QString indentText() const;

    int width() const;
    void setWidth(int);

    bool useTabs() const;
    void setUseTabs(bool);

    void setLanguage(const QString &language);

    bool shouldAutoIndentOnEvent(QKeyEvent *event) const;
    bool shouldUnindentWithBackspace(const QTextCursor &cursor) const;
#if 0
    void autoIndentBlock(QTextBlock block, QChar typedKey) const;
#endif
    void indentBlock(QTextBlock block, int column, QChar typedKey) const;

  public slots:
    void onShortcutIndentAfterCursor(QTextCursor cursor) const;
    void onShortcutUnindentWithBackspace(QTextCursor &cursor) const;

  private:
    std::unique_ptr<IndentAlgImpl> alg_;
    bool useTabs_;
    int width_;
    QString language_;
};

} // namespace Qutepart

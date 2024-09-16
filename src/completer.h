#pragma once

#include <QObject>
#include <QSet>
#include <QTimer>
#include <memory>

namespace Qutepart {

class Qutepart;

class CompletionList;
class CompletionModel;

class Completer : public QObject {
    Q_OBJECT

  public:
    Completer(Qutepart *qpart);
    ~Completer();

    void setKeywords(const QSet<QString> &keywords);

    bool isVisible() const;
    bool invokeCompletionIfAvailable(bool requestedByUser);

  public slots:
    void invokeCompletion();

  private slots:
    void onTextChanged();
    void onModificationChanged(bool modified);
    void onCompletionListItemSelected(int index);
    void onCompletionListTabPressed();

  private:
    void setCustomCompletions(const QSet<QString> &wordSet);
    void updateWordSet();
    bool shouldShowModel(CompletionModel *model, bool forceShow);
    void createWidget(CompletionModel *model);
    void closeCompletion();
    QString getWordBeforeCursor() const;
    QString getWordAfterCursor() const;

    Qutepart *qpart_;
    std::unique_ptr<CompletionList> widget_;
    bool completionOpenedManually_;
    QSet<QString> keywords_;
    QSet<QString> customCompletions_;
    QSet<QString> wordSet_;
    QTimer updateWordSetTimer_;
};

} // namespace Qutepart

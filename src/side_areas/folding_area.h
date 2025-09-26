#pragma once

#include <QWidget>
#include <QTextBlock>

namespace Qutepart {

class Qutepart;

class FoldingArea : public QWidget {
    Q_OBJECT

public:
    explicit FoldingArea(Qutepart *editor);

    QSize sizeHint() const override;
    int widthHint() const;

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void leaveEvent(QEvent *event) override;

public slots:
    void onUpdateRequest(const QRect &rect, int dy);

signals:
    void foldClicked(int lineNumber);

private:
    QTextBlock blockAt(const QPoint& pos) const;
    Qutepart *editor_;
};

} // namespace Qutepart
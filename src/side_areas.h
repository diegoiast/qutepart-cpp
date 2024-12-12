#include <QPlainTextEdit>
#include <QWidget>

namespace Qutepart {

class Qutepart;

class SideArea : public QWidget {
    Q_OBJECT

  public:
    SideArea(Qutepart *textEdit);

  private slots:
    void onTextEditUpdateRequest(const QRect &rect, int dy);

  protected:
    Qutepart *qpart_;

    virtual void updateWidth(){};
};

class LineNumberArea : public SideArea {
    Q_OBJECT

  public:
    LineNumberArea(Qutepart *textEdit);

    int widthHint() const;

  signals:
    void widthChanged();

  private slots:
    void updateWidth() override;

  private:
    void paintEvent(QPaintEvent *event) override;
    void changeEvent(QEvent *event) override;

    int desiredWidth_;
};

class MarkArea : public SideArea {
  public:
    MarkArea(Qutepart *qpart);
    int widthHint() const;

  private:
    QPixmap loadIcon(const QString &name) const;

    void paintEvent(QPaintEvent *event) override;

#if 0 // TODO
    void mouseMoveEvent(QMouseEvent* event);
#endif

    QPixmap bookmarkPixmap_;
};

class Minimap : public SideArea {
public:
    Minimap(Qutepart *textEdit);

    int widthHint() const;

  private:
    void mouseMoveEvent(QMouseEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void paintEvent(QPaintEvent *event) override;

    QFont minimapFont() const;
    void updateScroll(const QPoint &pos);
    void drawMinimapText(QPainter *painter, bool simple);

    bool isDragging = false;
    const int lineHeight = 3;
    const int charWidth = 3;

};

} // namespace Qutepart

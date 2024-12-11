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
    QFont minimapFont() const;
    void mousePressEvent(QMouseEvent *event) override;
    void paintEvent(QPaintEvent *event) override;

    void drawMinimapText(QPainter *painter, bool simple);
};

} // namespace Qutepart

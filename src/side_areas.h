/*
 * Copyright (C) 2018-2023 Andrei Kopats
 * Copyright (C) 2023-...  Diego Iastrubni <diegoiast@gmail.com>
 * SPDX-License-Identifier: MIT
 */

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
    virtual void wheelEvent(QWheelEvent *event) override;
    virtual void mouseMoveEvent(QMouseEvent *event) override;
    virtual void updateWidth() {}

    Qutepart *qpart_;
    int lastHoeveredLine = -1;
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
    virtual void paintEvent(QPaintEvent *event) override;
    virtual void changeEvent(QEvent *event) override;

    int desiredWidth_;
};

class MarkArea : public SideArea {
  public:
    MarkArea(Qutepart *qpart);
    int widthHint() const;
    virtual void changeEvent(QEvent *event) override;

  private:
    virtual void paintEvent(QPaintEvent *event) override;

    QIcon bookmarkPixmap_;
    QHash<QString, QPixmap> scaledIconCache;

    QPixmap getCachedIcon(QIcon icon, int targetSize, QHash<QString, QPixmap> &cache);
    QPixmap getCachedPixmap(QPixmap pixmap, int targetSize, QHash<QString, QPixmap> &cache);
};

class Minimap : public SideArea {
    Q_OBJECT
  public:
    Minimap(Qutepart *textEdit);

    int widthHint() const;
    void invalidateCache();

  protected:
    virtual void mouseMoveEvent(QMouseEvent *event) override;
    virtual void mousePressEvent(QMouseEvent *event) override;
    virtual void mouseReleaseEvent(QMouseEvent *event) override;
    virtual void paintEvent(QPaintEvent *event) override;

  private:
    QFont minimapFont() const;
    void updateScroll(const QPoint &pos);
    void drawMinimapText(QPainter *painter, bool simple);
    int visibleLineCount() const;
    int visibleViewportStartIndex() const;
    int viewportLineCount() const;
    int minimapOffsetForStart(int startIndex) const;
    QRect viewportRect() const;
    void ensureCache() const;
    int blockNumberForVisibleIndex(int idx) const;
    int visibleIndexForBlock(int blockNumber) const;

    bool isDragging = false;
    int dragOffset = 0;
    bool dragCenter = false;
    const int lineHeight = 3;
    const int charWidth = 3;

    mutable QVector<QTextBlock> visibleCache_;
    mutable bool cacheDirty_ = true;
    mutable int cachedBlockCount_ = -1;
};

class FoldingArea : public SideArea {
    Q_OBJECT

  public:
    explicit FoldingArea(Qutepart *editor);

    int widthHint() const;

  signals:
    void foldClicked(int lineNumber);

  protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;

  private:
    QTextBlock blockAt(const QPoint &pos) const;
    bool m_debugFolding = false;
};

} // namespace Qutepart

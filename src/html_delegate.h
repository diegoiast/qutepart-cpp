#pragma once

#include <QStyledItemDelegate>

namespace Qutepart {


/* QStyledItemDelegate implementation. Draws HTML

http://stackoverflow.com/questions/1956542/how-to-make-item-view-render-rich-html-text-in-qt/1956781#1956781
*/
class HTMLDelegate: public QStyledItemDelegate {
public:
    using QStyledItemDelegate::QStyledItemDelegate;

    void paint(
        QPainter *painter,
        const QStyleOptionViewItem& option,
        const QModelIndex& index) const override;

    virtual QSize sizeHint(const QStyleOptionViewItem &option,
                   const QModelIndex &index) const override;
};

}  // namespace Qutepart

/*
 * Copyright (C) 2018-2023 Andrei Kopats
 * Copyright (C) 2023-...  Diego Iastrubni <diegoiast@gmail.com>
 * SPDX-License-Identifier: MIT
 */

#include <map>
#include <string>

#include <QAbstractTextDocumentLayout>
#include <QApplication>
#include <QPainter>
#include <QTextDocument>

#include "html_delegate.h"

namespace Qutepart {

namespace {

std::map<QChar, QString> HTML_ESCAPE_TABLE = {
    {'&', "&amp;"},
    {'"', "&quot;"},
    {'\'', "&apos;"},
    {'>', "&gt;"},
    {'<', "&lt;"},
    {' ', "&nbsp;"},
    {'\t', "&nbsp;&nbsp;&nbsp;&nbsp;"},
};

#if 0 // FIXME not used. Remove?
// Replace special HTML symbols with escase sequences
QString htmlEscape(const QString& text) {
    QString result;
    for (auto ch: text) {
        auto replacement = HTML_ESCAPE_TABLE.find(ch);
        if (replacement == HTML_ESCAPE_TABLE.end()) {
            result += ch;
        } else {
            result += replacement->second;
        }
    }

    return result;
}
#endif

} // anonymous namespace

void HTMLDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
                         const QModelIndex &index) const {
    QStyleOptionViewItem myOption(option);
    myOption.state &= ~QStyle::State_HasFocus; // never draw focus rect

    QStyleOptionViewItem options(myOption);
    initStyleOption(&options, index);

    QStyle *style = nullptr;
    if (options.widget == nullptr) {
        style = QApplication::style();
    } else {
        style = options.widget->style();
    }

    QTextDocument doc;
    doc.setDocumentMargin(1);
    doc.setHtml(options.text);

    if (options.widget != nullptr) {
        doc.setDefaultFont(options.widget->font());
    }
    // bad long (multiline) strings processing
    // doc.setTextWidth(options.rect.width())

    options.text = "";
    style->drawControl(QStyle::CE_ItemViewItem, &options, painter);

    QAbstractTextDocumentLayout::PaintContext ctx;

    // Highlighting text if item is selected
    if (myOption.state & QStyle::State_Selected) {
        ctx.palette.setColor(QPalette::Text,
                             myOption.palette.color(QPalette::Active, QPalette::HighlightedText));
    }

    QRect textRect = style->subElementRect(QStyle::SE_ItemViewItemText, &options);
    painter->save();
    painter->translate(textRect.topLeft());

    /* Original example contained line
        painter.setClipRect(textRect.translated(-textRect.topLeft()))
    but text is drawn clipped with it on kubuntu 12.04
    */

    doc.documentLayout()->draw(painter, ctx);

    painter->restore();
}

QSize HTMLDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const {
    QStyleOptionViewItem options(option);
    initStyleOption(&options, index);

    QTextDocument doc;
    doc.setDocumentMargin(1);
    //  bad long (multiline) strings processing
    //  doc.setTextWidth(options.rect.width())
    doc.setHtml(options.text);
    return QSize(doc.idealWidth(), QStyledItemDelegate::sizeHint(option, index).height());
}

} // namespace Qutepart

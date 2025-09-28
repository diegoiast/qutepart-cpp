/*
 * Copyright (C) 2018-2023 Andrei Kopats
 * Copyright (C) 2023-...  Diego Iastrubni <diegoiast@gmail.com>
 * SPDX-License-Identifier: MIT
 */

#include <QDebug>
#include <QObject>
#include <QTest>

#include "qutepart.h"
#include "text_block_user_data.h"

class Test : public QObject {
    Q_OBJECT

  private slots:
    void TextCursorPosition() {
        Qutepart::Qutepart qutepart;
        qutepart.setPlainText("one\ntwo\nthree\nfour");
        qutepart.goTo(2);
        QCOMPARE(qutepart.textCursorPosition(), Qutepart::TextCursorPosition(2, 0));

        qutepart.goTo({2, 1});
        QCOMPARE(qutepart.textCursorPosition(), Qutepart::TextCursorPosition(2, 1));
    }

    void Folding() {
        // TODO: should be in test initialization, not here
        Q_INIT_RESOURCE(qutepart_syntax_files);

        Qutepart::Qutepart qutepart;
        qutepart.setPlainText("{\n    {\n    }\n}");
        qutepart.setHighlighter("cpp.xml");

        QTextBlock block = qutepart.document()->findBlockByNumber(0);
        auto data = static_cast<Qutepart::TextBlockUserData *>(block.userData());
        data->folding.folded = true;

        QVector<int> foldedLines = qutepart.getFoldedLines();
        QCOMPARE(foldedLines.size(), 1);
        QCOMPARE(foldedLines[0], 0);

        qutepart.setFoldedLines({1});
        foldedLines = qutepart.getFoldedLines();
        QCOMPARE(foldedLines.size(), 1);
        QCOMPARE(foldedLines[0], 1);
    }
};

QTEST_MAIN(Test)
#include "test_api.moc"

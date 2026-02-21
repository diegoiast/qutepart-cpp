/*
 * Copyright (C) 2018-2023 Andrei Kopats
 * Copyright (C) 2023-...  Diego Iastrubni <diegoiast@gmail.com>
 * SPDX-License-Identifier: MIT
 */

#include <QDebug>
#include <QObject>
#include <QStyle>
#include <QTest>

#include "qutepart.h"
#include "text_block_user_data.h"
#include "theme.h"

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

    void ThemeSwitching() {
        Q_INIT_RESOURCE(qutepart_syntax_files);
        Q_INIT_RESOURCE(qutepart_theme_data);

        Qutepart::Qutepart qutepart;

        // Initial state: system theme (nullptr)
        // Viewport palette should match widget palette
        QCOMPARE(qutepart.viewport()->palette().color(QPalette::Base),
                 qutepart.palette().color(QPalette::Base));

        // Set dark theme
        Qutepart::Theme darkTheme;
        if (darkTheme.loadTheme(":/qutepart/themes/atom-one-dark.theme")) {
            qutepart.setTheme(&darkTheme);
            // Check if background changed. Atom One Dark background is #282c34
            QColor darkBg("#282c34");
            // Use name() comparison to avoid fuzziness, though exact match expected
            QCOMPARE(qutepart.viewport()->palette().color(QPalette::Base).name(), darkBg.name());

            // Now set to nullptr (system theme)
            qutepart.setTheme(nullptr);

            // Viewport background should NOT be dark anymore.
            // It should be the system default.
            QCOMPARE(qutepart.viewport()->palette().color(QPalette::Base).name(),
                     qutepart.style()->standardPalette().color(QPalette::Base).name());
        } else {
            QSKIP("Could not load atom-one-dark.theme from resources");
        }
    }

    void testThemeToNullptr() {
        Qutepart::Qutepart qutepart;

        // 1. Set a theme
        Q_INIT_RESOURCE(qutepart_theme_data);
        Qutepart::Theme darkTheme;
        if (darkTheme.loadTheme(":/qutepart/themes/atom-one-dark.theme")) {
            qutepart.setTheme(&darkTheme);
            QCOMPARE(qutepart.viewport()->palette().color(QPalette::Base).name(),
                     QColor("#282c34").name());
        } else {
            QSKIP("Could not load theme");
        }

        // 2. Set theme to nullptr. It should go back to system palette.
        qutepart.setTheme(nullptr);

        // If the bug is fixed, it will go back to system palette.
        QCOMPARE(qutepart.viewport()->palette().color(QPalette::Base).name(),
                 qutepart.style()->standardPalette().color(QPalette::Base).name());
    }

    void PaletteChange() {
        Qutepart::Qutepart qutepart;

        // 1. Initial palette (Blue)
        QPalette p = QApplication::palette();
        p.setColor(QPalette::Highlight, Qt::blue);
        QApplication::setPalette(p);
        
        QEvent paletteEvent1(QEvent::PaletteChange);
        QApplication::sendEvent(&qutepart, &paletteEvent1);

        // Viewport should have customized 180 alpha highlight
        QCOMPARE(qutepart.viewport()->palette().color(QPalette::Highlight).alpha(), 180);
        QCOMPARE(qutepart.viewport()->palette().brush(QPalette::HighlightedText).style(), Qt::NoBrush);
        
        // 2. Change to Red
        p.setColor(QPalette::Highlight, Qt::red);
        QApplication::setPalette(p);
        
        QEvent paletteEvent2(QEvent::PaletteChange);
        QApplication::sendEvent(&qutepart, &paletteEvent2);

        QCOMPARE(qutepart.viewport()->palette().color(QPalette::Highlight).alpha(), 180);
        QCOMPARE(qutepart.viewport()->palette().brush(QPalette::HighlightedText).style(), Qt::NoBrush);
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

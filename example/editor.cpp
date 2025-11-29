/*
 * Copyright (C) 2018-2023 Andrei Kopats
 * Copyright (C) 2023-...  Diego Iastrubni <diegoiast@gmail.com>
 * SPDX-License-Identifier: MIT
 */

#include <QApplication>
#include <QByteArray>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QMainWindow>
#include <QMenu>
#include <QMenuBar>
#include <QString>
#include <QXmlStreamReader>

#include <Sonnet/Speller>
#include <Sonnet/Highlighter>

#include "qutepart.h"
#include "theme.h"

// a C comment
/*
 * multiline comment - other features
 */

/**
 * \brief doxygen comment
 *
 * This shows how more complex comments are supported
 */

#if 0
this will also be displayed as a comment
#endif

// note the minimap on the side

bool openFile(const QString &filePath, Qutepart::Qutepart *qutepart) {
    QFile file(filePath);
    if (file.exists()) {
        Qutepart::LangInfo langInfo = Qutepart::chooseLanguage(QString(), QString(), filePath);
        if (langInfo.isValid()) {
            qutepart->setHighlighter(langInfo.id);
            qutepart->setIndentAlgorithm(langInfo.indentAlg);
        }

        file.open(QIODevice::ReadOnly);
        QByteArray data = file.readAll();
        QString text = QString::fromUtf8(data);
        qutepart->setPlainText(text);
    } else {
        qWarning() << "File does not exist" << filePath;
        return false;
    }

    return true;
}

auto countTrailingSpaces(const QString &str) -> int {
    auto count = 0;
    for (auto i = str.length(); i != 0; --i) {
        if (str[i - 1] == ' ') {
            ++count;
        } else {
            break;
        }
    }
    return count;
}

auto countLeadingSpaces(const QString &str) -> int {
    auto count = 0;
    for (auto i = 0; i < str.length(); ++i) {
        if (str[i] == ' ') {
            ++count;
        } else {
            break;
        }
    }
    return count;
}

void initMenuBar(QMenuBar *menuBar, Qutepart::Qutepart *qutepart) {
    QMenu *editMenu = menuBar->addMenu("Edit");
    editMenu->addAction(qutepart->increaseIndentAction());
    editMenu->addAction(qutepart->decreaseIndentAction());

    QMenu *viewMenu = menuBar->addMenu("View");

    viewMenu->addSection("Visuals");
    {
        auto minimapAction = new QAction(viewMenu);
        minimapAction->setText("Show/hide minimap");
        minimapAction->setCheckable(true);
        minimapAction->setChecked(qutepart->minimapVisible());
        QObject::connect(minimapAction, &QAction::triggered, minimapAction,
                         [minimapAction, qutepart]() {
                             qutepart->setMinimapVisible(minimapAction->isChecked());
                         });
        viewMenu->addAction(minimapAction);
        viewMenu->addAction(minimapAction);
        viewMenu->addAction(qutepart->zoomInAction());
        viewMenu->addAction(qutepart->zoomOutAction());
    }

    viewMenu->addSection("File modifications");
    {
        auto removeNotificationsAction = new QAction(viewMenu);
        removeNotificationsAction->setText("Remove modifications markings");
        QObject::connect(removeNotificationsAction, &QAction::triggered, removeNotificationsAction,
                         [qutepart]() { qutepart->removeModifications(); });
        viewMenu->addAction(removeNotificationsAction);
    }

    viewMenu->addSection("Markings");
    {
        auto addMarkingsAction = new QAction(viewMenu);
        addMarkingsAction->setText("Set markings on code");
        QObject::connect(addMarkingsAction, &QAction::triggered, addMarkingsAction, [qutepart]() {
            for (auto line : qutepart->lines()) {
                auto s1 = countLeadingSpaces(line.text());
                auto s2 = countTrailingSpaces(line.text());
                auto lineNumber = line.lineNumber();

                if ((s2 - s1) > 2) {
                    qutepart->setLineWarning(lineNumber, true);
                    qutepart->setLineMessage(
                        lineNumber,
                        QString("Line %1 has %2 spaces at the end!!!!! That's too much!")
                            .arg(lineNumber)
                            .arg(s2));
                    qutepart->repaint();
                }
                // if ((s1 - s2) > 5) {
                //     auto flags = qutepart->getLineFlags(lineNumber);
                //     flags |= Qutepart::SidebarFlag::Error;
                //     qutepart->setLineFlags(lineNumber, flags);
                //     qutepart->setLineMessage(lineNumber, QString("Line %1 has %2 spaces at the
                //     start").arg(lineNumber).arg(s1)); qutepart->repaint();
                // }
                if (lineNumber == 10) {
                    qutepart->setLineWarning(lineNumber, true);
                    qutepart->setLineMessage(lineNumber, QString("This is an warning message"));
                    qutepart->repaint();
                }
                if (lineNumber == 11) {
                    qutepart->setLineError(lineNumber, true);
                    qutepart->setLineMessage(lineNumber, QString("This is an error message"));
                    qutepart->repaint();
                }
                if (lineNumber == 12) {
                    qutepart->setLineInfo(lineNumber, true);
                    qutepart->setLineMessage(lineNumber, QString("Lucky 13 (this is info)"));
                    qutepart->repaint();
                }

                if (lineNumber == 19) {
                    qutepart->setLineWarning(lineNumber, true);
                    qutepart->setLineInfo(lineNumber, true);
                    qutepart->setLineMessage(lineNumber, QString("This is warning+info"));
                    qutepart->repaint();
                }
                if (lineNumber == 22) {
                    qutepart->setLineWarning(lineNumber, true);
                    qutepart->setLineError(lineNumber, true);
                    qutepart->setLineMessage(lineNumber, QString("This is warning+error"));
                    qutepart->repaint();
                }
                if (lineNumber == 29) {
                    qutepart->setLineError(lineNumber, true);
                    qutepart->setLineInfo(lineNumber, true);
                    qutepart->setLineMessage(lineNumber, QString("This is info+error"));
                    qutepart->repaint();
                }
            }
        });
        viewMenu->addAction(addMarkingsAction);

        auto clearMarkingsAction = new QAction(viewMenu);
        clearMarkingsAction->setText("Clear side markings");
        QObject::connect(clearMarkingsAction, &QAction::triggered, clearMarkingsAction,
                         [clearMarkingsAction, qutepart]() { qutepart->removeMetaData(); });
        viewMenu->addAction(clearMarkingsAction);

        addMarkingsAction->trigger();
    }

    QMenu *navMenu = menuBar->addMenu("Navigation");
    navMenu->addAction(qutepart->toggleBookmarkAction());
    navMenu->addAction(qutepart->prevBookmarkAction());
    navMenu->addAction(qutepart->nextBookmarkAction());
    navMenu->addAction(qutepart->findMatchingBracketAction());

    navMenu->addSeparator();
    navMenu->addAction(qutepart->scrollDownAction());
    navMenu->addAction(qutepart->scrollUpAction());

    QMenu *linesMenu = menuBar->addMenu("Lines");

    linesMenu->addAction(qutepart->duplicateSelectionAction());
    linesMenu->addSeparator();

    linesMenu->addAction(qutepart->moveLineUpAction());
    linesMenu->addAction(qutepart->moveLineDownAction());
    linesMenu->addSeparator();

    linesMenu->addAction(qutepart->deleteLineAction());
    linesMenu->addSeparator();

    linesMenu->addAction(qutepart->cutLineAction());
    linesMenu->addAction(qutepart->copyLineAction());
    linesMenu->addAction(qutepart->pasteLineAction());

    linesMenu->addSeparator();
    linesMenu->addAction(qutepart->joinLinesAction());
}

QMainWindow *createMainWindow(Qutepart::Qutepart *qutepart) {
    QMainWindow *window = new QMainWindow();
    window->resize(800, 600);

    window->setCentralWidget(qutepart);

    QMenuBar *menuBar = window->menuBar();
    initMenuBar(menuBar, qutepart);

    return window;
}

int main(int argc, char **argv) {
    Q_INIT_RESOURCE(qutepart_syntax_files);
    Q_INIT_RESOURCE(qutepart_theme_data);
    QApplication app(argc, argv);

    // put cursor over a work, to highlight it on the document
    Qutepart::Theme *theme = new Qutepart::Theme;
    Qutepart::Qutepart qutepart;
    // selection will also highlight, but now its case sensitive
    qutepart.setMarkCurrentWord(true);

    theme->loadTheme(":/qutepart/themes/github-light.theme");
    qutepart.setTheme(theme);

    QFont font = qutepart.font();
    font.setPointSize(12);
    // alt + arrow moves line
    font.setFamily("Monospace");
    qutepart.setFont(font);

    auto s = new Sonnet::Highlighter(&qutepart);
    qDebug() << "Sonnet active: " << s->isActive();

    // toggle comment a line, block
    auto filePath = QString(":/qutepart/syntax/c.xml");
    if (argc > 1) {
        filePath = argv[1];
    }
    // matching brackets
    if (!openFile(filePath, &qutepart)) {
        return -1;
    }

    QMainWindow *window = createMainWindow(&qutepart);
    window->show();

    return app.exec();
}

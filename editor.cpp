#include <stdio.h>

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

#include "qutepart.h"
#include "theme.h"

bool openFile(const QString &filePath, Qutepart::Qutepart *qutepart, const Qutepart::Theme *theme) {
    QFile file(filePath);
    if (file.exists()) {
        Qutepart::LangInfo langInfo = Qutepart::chooseLanguage(QString(), QString(), filePath);
        if (langInfo.isValid()) {
            qutepart->setHighlighter(langInfo.id, theme);
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

void initMenuBar(QMenuBar *menuBar, Qutepart::Qutepart *qutepart) {
    QMenu *editMenu = menuBar->addMenu("Edit");
    editMenu->addAction(qutepart->increaseIndentAction());
    editMenu->addAction(qutepart->decreaseIndentAction());

    QMenu *viewMenu = menuBar->addMenu("View");
    viewMenu->addAction(qutepart->zoomInAction());
    viewMenu->addAction(qutepart->zoomOutAction());

    QMenu *navMenu = menuBar->addMenu("Navigation");
    navMenu->addAction(qutepart->toggleBookmarkAction());
    navMenu->addAction(qutepart->prevBookmarkAction());
    navMenu->addAction(qutepart->nextBookmarkAction());

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

    Qutepart::Theme *theme = new Qutepart::Theme;
    Qutepart::Qutepart qutepart;

    theme->loadTheme(":/qutepart/themes/github-light.theme");
    qutepart.setTheme(theme);

    QFont font = qutepart.font();
    font.setPointSize(12);
    font.setFamily("Monospace");
    qutepart.setFont(font);

    if (argc > 1) {
        QString filePath = argv[1];
        filePath = "editor.cpp";
        // filePath = ":/qutepart/syntax/c.xml";
        if (!openFile(filePath, &qutepart, theme)) {
            return -1;
        }
    }

    QMainWindow *window = createMainWindow(&qutepart);
    window->show();

    return app.exec();
}

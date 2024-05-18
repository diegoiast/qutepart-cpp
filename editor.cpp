#include <stdio.h>

#include <QApplication>
#include <QMainWindow>
#include <QMenu>
#include <QMenuBar>
#include <QXmlStreamReader>
#include <QFile>
#include <QDebug>
#include <QDir>
#include <QString>
#include <QByteArray>
#include <QTextCodec>

#include "qutepart.h"


bool openFile(const QString& filePath, Qutepart::Qutepart* qutepart) {
    QFile file(filePath);
    if (file.exists()) {
        Qutepart::LangInfo langInfo = Qutepart::chooseLanguage(
            QString(), QString(), filePath);
        if (langInfo.isValid()) {
            qutepart->setHighlighter(langInfo.id);
            qutepart->setIndentAlgorithm(langInfo.indentAlg);
        }

        file.open(QIODevice::ReadOnly);
        QByteArray data = file.readAll();
        QString text = QTextCodec::codecForUtfText(data, QTextCodec::codecForName("UTF-8"))->toUnicode(data);
        qutepart->setPlainText(text);
    } else {
        qWarning() << "File does not exist" << filePath;
        return false;
    }

    return true;
}

void initMenuBar(QMenuBar* menuBar, Qutepart::Qutepart* qutepart) {
    QMenu* editMenu = menuBar->addMenu("Edit");
    editMenu->addAction(qutepart->increaseIndentAction());
    editMenu->addAction(qutepart->decreaseIndentAction());

    QMenu* viewMenu = menuBar->addMenu("View");
    viewMenu->addAction(qutepart->zoomInAction());
    viewMenu->addAction(qutepart->zoomOutAction());

    QMenu* navMenu = menuBar->addMenu("Navigation");
    navMenu->addAction(qutepart->toggleBookmarkAction());
    navMenu->addAction(qutepart->prevBookmarkAction());
    navMenu->addAction(qutepart->nextBookmarkAction());

    navMenu->addSeparator();
    navMenu->addAction(qutepart->scrollDownAction());
    navMenu->addAction(qutepart->scrollUpAction());

    QMenu* linesMenu = menuBar->addMenu("Lines");

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

QMainWindow* createMainWindow(Qutepart::Qutepart* qutepart) {
    QMainWindow* window = new QMainWindow();
    window->resize(800, 600);

    window->setCentralWidget(qutepart);

    QMenuBar* menuBar = window->menuBar();
    initMenuBar(menuBar, qutepart);

    return window;
}

int main(int argc, char** argv) {
    Q_INIT_RESOURCE(qutepart_syntax_files);
    QApplication app(argc, argv);

    Qutepart::Qutepart qutepart;

    QFont font = qutepart.font();
    font.setPointSize(12);
    font.setFamily("Monospace");
    qutepart.setFont(font);

    if (argc > 1) {
        QString filePath = argv[1];
        if ( ! openFile(filePath, &qutepart)) {
            return -1;
        }
    }

    QMainWindow* window = createMainWindow(&qutepart);
    window->show();

    return app.exec();
}

#include <QObject>
#include <QTest>

#include "bracket_highlighter.h"
#include "qutepart.h"
#include "text_pos.h"

class Test : public QObject {
    Q_OBJECT

  private slots:
    void BracketHighlighter() {
        Qutepart::Qutepart qpart;
        qpart.setPlainText("{\n    new OpenFileList(mainWindow_, this);\n\n}");
        Qutepart::TextPosition pos(qpart.document()->firstBlock(), 0);
        Qutepart::BracketHighlighter bh;
        QList<QTextEdit::ExtraSelection> selections = bh.extraSelections(pos);

        QCOMPARE(selections.size(), 2); // if not empty - managed to find a pair
        QCOMPARE(selections[0].cursor.blockNumber(), 0);
        QCOMPARE(selections[0].cursor.positionInBlock(), 1);
        QCOMPARE(selections[1].cursor.blockNumber(), 3);
        QCOMPARE(selections[1].cursor.positionInBlock(), 1);
    }
};

QTEST_MAIN(Test)
#include "test_bracket_highlighter.moc"

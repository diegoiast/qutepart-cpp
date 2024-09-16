#include <memory>

#include <QDebug>
#include <QObject>
#include <QTest>

#include "char_iterator.h"
#include "qutepart.h"

class Test : public QObject {
    Q_OBJECT

  private slots:
    void DataDrivenTest_data() {
        QTest::addColumn<QString>("text");
        QTest::addColumn<int>("blockNumber");
        QTest::addColumn<int>("column");
        QTest::addColumn<bool>("forward");
        QTest::addColumn<QString>("expected");

        QTest::newRow("forward from start")
            << "one\ntwo\nthree\nfour" << 0 << 0 << true << "onetwothreefour";

        QTest::newRow("forward from middle")
            << "one\ntwo\nthree\nfour" << 1 << 2 << true << "othreefour";

        QTest::newRow("backward from end")
            << "one\ntwo\nthree\nfour" << 3 << 3 << false << "ruofeerhtowteno";

        QTest::newRow("backward from middle")
            << "one\ntwo\nthree\nfour" << 2 << 1 << false << "htowteno";

        QTest::newRow("with new lines")
            << "{\n    new OpenFileList(mainWindow_, this);\n\n}" << 0 << 0 << true
            << "{    new OpenFileList(mainWindow_, this);}";

        QTest::newRow("with new lines backward")
            << "{\n    new OpenFileList(mainWindow_, this);\n\n}" << 3 << 0 << false
            << "};)siht ,_wodniWniam(tsiLeliFnepO wen    {";
    }

    void DataDrivenTest() {
        QFETCH(QString, text);
        QFETCH(int, blockNumber);
        QFETCH(int, column);
        QFETCH(bool, forward);
        QFETCH(QString, expected);

        Qutepart::Qutepart qpart(nullptr, text);

        Qutepart::TextPosition pos(qpart.document()->findBlockByNumber(blockNumber), column);
        std::unique_ptr<Qutepart::CharIterator> it;
        if (forward) {
            it = std::make_unique<Qutepart::ForwardCharIterator>(pos);
        } else {
            it = std::make_unique<Qutepart::BackwardCharIterator>(pos);
        }

        QString accumulatedText;

        while (!it->atEnd()) {
            accumulatedText += it->step();
        }

        QCOMPARE(accumulatedText, expected);
    }
};

QTEST_MAIN(Test)
#include "test_char_iterator.moc"

#include <QtTest>
#include <QApplication>
#include <QScrollBar>
#include "qutepart/qutepart.h"
#include "side_areas.h"

using namespace Qutepart;

class TestMinimapSmall : public QObject {
    Q_OBJECT
private slots:
    void testDragReachesEndOfTextOnSmallDoc() {
        auto *qpart = new Qutepart::Qutepart();
        qpart->resize(1200, 800);
        qpart->show();
        QVERIFY(QTest::qWaitForWindowExposed(qpart));

        // Small doc: the minimap text does not fill the widget height
        auto text = QString();
        for (auto i = 0; i < 150; ++i) {
            text += QString("line %1\n").arg(i);
        }
        qpart->setPlainText(text);
        qpart->setMinimapVisible(true);
        QApplication::processEvents();

        auto *mini = qpart->findChild<Minimap*>();
        QVERIFY(mini != nullptr);
        mini->resize(150, 800);
        QApplication::processEvents();

        auto visibleH = mini->height();
        auto contentH = 150 * 3;
        qDebug() << "visibleH" << visibleH << "contentH" << contentH;
        QVERIFY2(contentH < visibleH,
                 qPrintable(QString("small doc contentH %1 should be below widget %2")
                                .arg(contentH).arg(visibleH)));

        auto viewportLines = qMax(1, qpart->viewport()->height() / qpart->fontMetrics().height());
        QVERIFY2(150 > viewportLines,
                 qPrintable(QString("doc must be scrollable: %1 lines vs %2 viewport lines")
                                .arg(150).arg(viewportLines)));
        auto maxStart = 150 - viewportLines;
        auto viewportHeight = viewportLines * 3;

        // Press inside the thumb at its very top (thumb starts at y=0 for a new doc)
        auto pressY = 3;
        auto pressPos = QPoint(mini->width() / 2, pressY);
        QTest::mousePress(mini, Qt::LeftButton, Qt::NoModifier, pressPos);
        QApplication::processEvents();

        // Drag to the y where the end of the text is: maxViewportY + dragOffset
        // (contentH - viewportHeight) corresponds to maxStart with the fix.
        auto maxViewportY = contentH - viewportHeight;
        auto endY = maxViewportY + pressY;
        QVERIFY2(endY <= contentH,
                 qPrintable(QString("drag point %1 should be inside text extent %2")
                                .arg(endY).arg(contentH)));
        auto endPos = QPoint(mini->width() / 2, endY);
        QTest::mouseMove(mini, endPos);
        QApplication::processEvents();
        QTest::mouseRelease(mini, Qt::LeftButton, Qt::NoModifier, endPos);
        QApplication::processEvents();

        auto sb = qpart->verticalScrollBar()->value();
        qDebug() << "endY" << endY << "sb" << sb << "expected maxStart" << maxStart;
        QVERIFY2(qAbs(sb - maxStart) < 5,
                 qPrintable(QString("drag to text end sb %1, expected %2")
                                .arg(sb).arg(maxStart)));

        delete qpart;
    }
};

QTEST_MAIN(TestMinimapSmall)
#include "test_minimap_small.moc"
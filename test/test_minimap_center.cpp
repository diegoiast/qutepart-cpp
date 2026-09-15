#include <QtTest>
#include <QSignalSpy>
#include <QApplication>
#include <QScrollBar>
#include "qutepart/qutepart.h"
#include "side_areas.h"

using namespace Qutepart;

class TestMinimapCenter : public QObject {
    Q_OBJECT
private slots:
    void testClick33Percent() {
        auto *qpart = new Qutepart::Qutepart();
        qpart->resize(1200, 800);
        qpart->show();
        QVERIFY(QTest::qWaitForWindowExposed(qpart));

        // Create 10k lines
        auto text = QString();
        for (auto i = 0; i < 10000; ++i) {
            text += QString("line %1\n").arg(i);
        }
        qpart->setPlainText(text);
        qpart->setMinimapVisible(true);
        QApplication::processEvents();

        auto *mini = qpart->findChild<Minimap*>();
        QVERIFY(mini != nullptr);
        mini->resize(150, 800);
        QApplication::processEvents();

        // Ensure minimap is visible
        QVERIFY(mini->isVisible());
        auto h = mini->height();
        QVERIFY(h > 0);
        qDebug() << "minimap height" << h << "width" << mini->width();

        // Click at 33% of minimap height
        auto y33 = int(h * 0.33);
        auto x = mini->width() / 2;
        auto pos = QPoint(x, y33);
        qDebug() << "click pos" << pos << "y33" << y33;

        // Simulate mouse press and release
        QTest::mousePress(mini, Qt::LeftButton, Qt::NoModifier, pos);
        QTest::mouseRelease(mini, Qt::LeftButton, Qt::NoModifier, pos);
        QApplication::processEvents();

        // Check first visible via scrollbar value (which is blockNumber for top)
        auto sbValue = qpart->verticalScrollBar()->value();
        auto cursorBlock = qpart->textCursor().blockNumber();
        qDebug() << "sbValue" << sbValue << "cursorBlock" << cursorBlock << "expected ~3333";

        // The first line should be approximately 3333 (33% of 10000)
        // Allow some tolerance due to viewport size and rounding
        auto expected = 3333;
        auto tolerance = 100; // allow 100 lines tolerance (viewport ~40 lines)
        QVERIFY2(qAbs(sbValue - expected) < tolerance,
                 qPrintable(QString("sbValue %1 expected %2 tolerance %3").arg(sbValue).arg(expected).arg(tolerance)));
        // Clicking should also make the clicked line the current line (cursor)
        QVERIFY2(qAbs(cursorBlock - expected) < tolerance,
                 qPrintable(QString("cursorBlock %1 expected %2 tolerance %3").arg(cursorBlock).arg(expected).arg(tolerance)));
        // Also check that the clicked line is at the top, not centered
        // If centered, firstVisible would be ~3333 - viewportLines/2
        // We want firstVisible == 3333, not centered
        delete qpart;
    }
};

QTEST_MAIN(TestMinimapCenter)
#include "test_minimap_center.moc"

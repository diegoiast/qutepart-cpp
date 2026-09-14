#include <QtTest>
#include <QApplication>
#include "qutepart/qutepart.h"
#include "side_areas.h"

using namespace Qutepart;

class TestMinimapLarge : public QObject {
    Q_OBJECT
private slots:
    void testLargeDocHidesMinimap() {
        auto *qpart = new Qutepart::Qutepart();
        qpart->resize(1200, 800);
        qpart->show();
        QVERIFY(QTest::qWaitForWindowExposed(qpart));

        // Start with small doc, minimap should be visible
        auto smallText = QString();
        for (auto i = 0; i < 100; ++i) smallText += QString("line %1\n").arg(i);
        qpart->setPlainText(smallText);
        qpart->setMinimapVisible(true);
        QApplication::processEvents();
        QVERIFY(qpart->minimapVisible());
        auto *mini = qpart->findChild<Qutepart::Minimap*>();
        QVERIFY(mini != nullptr);
        QVERIFY(mini->isVisible());

        // Now set large doc >20000 lines
        auto largeText = QString();
        for (auto i = 0; i < 20001; ++i) largeText += QString("line %1\n").arg(i);
        qpart->setPlainText(largeText);
        QApplication::processEvents();
        // After setting large doc, minimap should be considered not visible
        QVERIFY(!qpart->minimapVisible());
        // The widget should be hidden
        QVERIFY(!mini->isVisible());
        // Scrollbar should be visible (AlwaysOn) when minimap hidden for large doc
        QCOMPARE(qpart->verticalScrollBarPolicy(), Qt::ScrollBarAlwaysOn);

        // Try to enable minimap when large doc - should not enable
        qpart->setMinimapVisible(true);
        QApplication::processEvents();
        QVERIFY(!qpart->minimapVisible());
        QVERIFY(!mini->isVisible());

        // Reduce doc to small again, minimap should be able to be enabled
        qpart->setPlainText(smallText);
        QApplication::processEvents();
        // After reducing, updateViewport should be called via blockCountChanged, but minimap is still hidden
        // Need to explicitly enable again
        qpart->setMinimapVisible(true);
        QApplication::processEvents();
        QVERIFY(qpart->minimapVisible());
        // Note: need to get new mini pointer after re-creation
        auto *mini2 = qpart->findChild<Qutepart::Minimap*>();
        QVERIFY(mini2 != nullptr);
        QVERIFY(mini2->isVisible());

        delete qpart;
    }

    void testExactly20000StillVisible() {
        auto *qpart = new Qutepart::Qutepart();
        qpart->resize(1200, 800);
        qpart->show();
        QVERIFY(QTest::qWaitForWindowExposed(qpart));

        auto text = QString();
        for (auto i = 0; i < 20000; ++i) {
            if (i > 0) text += "\n";
            text += QString("line %1").arg(i);
        }
        qpart->setPlainText(text);
        QVERIFY(qpart->document()->blockCount() == 20000);
        qpart->setMinimapVisible(true);
        QApplication::processEvents();
        QVERIFY(qpart->minimapVisible());
        auto *mini = qpart->findChild<Qutepart::Minimap*>();
        QVERIFY(mini->isVisible());

        // One more line should hide (20001)
        text += "\nline 20000";
        qpart->setPlainText(text);
        QVERIFY(qpart->document()->blockCount() == 20001);
        QApplication::processEvents();
        QVERIFY(!qpart->minimapVisible());
        // mini may have been deleted or hidden, get new pointer
        auto *mini2 = qpart->findChild<Qutepart::Minimap*>();
        if (mini2) QVERIFY(!mini2->isVisible());

        delete qpart;
    }
};

QTEST_MAIN(TestMinimapLarge)
#include "test_minimap_large.moc"

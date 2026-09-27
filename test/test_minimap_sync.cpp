#include <QtTest>
#include <QApplication>
#include <QScrollBar>
#include "qutepart/qutepart.h"

#define private public
#define protected public
#include "side_areas.h"
#undef protected
#undef private

class TestMinimapSync : public QObject {
    Q_OBJECT
private slots:
    void testHiddenEnsureCacheDoesNotFreezeAfterShow() {
        auto *qpart = new Qutepart::Qutepart();
        qpart->resize(1200, 800);
        qpart->show();
        QVERIFY(QTest::qWaitForWindowExposed(qpart));

        auto text = QString();
        for (auto i = 0; i < 100; ++i) {
            if (i > 0)
                text += "\n";
            text += QString("line %1").arg(i);
        }
        qpart->setPlainText(text);
        qpart->setMinimapVisible(true);
        QApplication::processEvents();

        auto *mini = qpart->findChild<Qutepart::Minimap *>();
        QVERIFY(mini != nullptr);
        QCOMPARE(mini->visibleLineCount(), 100);
        QVERIFY(!mini->visibleCache_.isEmpty());

        // Hide without processEvents: updateViewport() would call show() again.
        mini->setVisible(false);
        QVERIFY(!mini->isVisible());

        // Triggers ensureCache() while !isVisible() — buggy path clears cache
        // and sets cacheDirty_=false, then refuses to rebuild.
        mini->blockNumberForVisibleIndex(0);
        QVERIFY(mini->visibleCache_.isEmpty());

        mini->setVisible(true);
        QVERIFY(mini->isVisible());

        // showEvent() also invalidates; undo that so we specifically assert
        // ensureCache() rebuilds an empty-but-not-dirty cache (the freeze bug).
        mini->cacheDirty_ = false;
        mini->cachedBlockCount_ = qpart->document()->blockCount();
        QVERIFY(mini->visibleCache_.isEmpty());

        // Must rebuild; broken code returns 0 forever after the freeze.
        QCOMPARE(mini->visibleLineCount(), 100);
        QVERIFY(!mini->visibleCache_.isEmpty());

        delete qpart;
    }

    void testMinimapDoesNotBitblitWithEditorDy() {
        // Line-number areas bitblit with the editor (1:1 pixels). The minimap is
        // a scaled overview — editor dy must not call QWidget::scroll, or the
        // image scrambles/inverts. Paint-rect observation is unreliable for
        // child widgets (offscreen full-repaints; xcb may emit no paint), so
        // assert the explicit policy hook instead.
        auto *qpart = new Qutepart::Qutepart();
        qpart->resize(1200, 800);
        qpart->show();
        QVERIFY(QTest::qWaitForWindowExposed(qpart));

        qpart->setPlainText("a\nb\nc\n");
        qpart->setMinimapVisible(true);
        QApplication::processEvents();

        auto *mini = qpart->findChild<Qutepart::Minimap *>();
        QVERIFY(mini != nullptr);
        QVERIFY2(!mini->usesEditorScrollBitblit(),
                 "Minimap must opt out of SideArea scroll(0, dy) bitblit");

        delete qpart;
    }
};

QTEST_MAIN(TestMinimapSync)
#include "test_minimap_sync.moc"

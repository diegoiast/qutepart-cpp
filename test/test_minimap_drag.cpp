#include <QtTest>
#include <QApplication>
#include <QScrollBar>
#include "qutepart/qutepart.h"
#include "side_areas.h"

class TestMinimapDrag : public QObject {
    Q_OBJECT
private slots:
    void testDragInsideThumbFollowsMouse() {
        auto *qpart = new Qutepart::Qutepart();
        qpart->resize(1200, 800);
        qpart->show();
        QVERIFY(QTest::qWaitForWindowExposed(qpart));

        auto text = QString();
        for (auto i = 0; i < 10000; ++i) {
            text += QString("line %1\n").arg(i);
        }
        qpart->setPlainText(text);
        qpart->setMinimapVisible(true);
        QApplication::processEvents();

        auto *mini = qpart->findChild<Qutepart::Minimap*>();
        QVERIFY(mini != nullptr);
        mini->resize(150, 800);
        QApplication::processEvents();

        QVERIFY(mini->isVisible());
        auto h = mini->height();
        auto w = mini->width();
        QVERIFY(h > 0);

        // Get initial viewport rect (thumb at top for new doc)
        auto initialRect = mini->findChild<QWidget*>(); // dummy to keep
        // Actually get viewportRect via minimap's method (need to make it public or use friend)
        // For test, we can just get the rect via minimap->viewportRect() if we make it public
        // Instead, we will use the minimap's geometry and assume thumb at top
        // For a new doc with first line 0, thumb should be at top (y=0)
        // Let's get the thumb rect via the minimap's viewportRect (need to expose)
        // We can call the private method via a hack: use the minimap's height and known viewportLines
        // Simpler: just test the drag logic by pressing at the center of the thumb

        // For a new doc, firstVisible should be 0, thumb at top
        auto sbBefore = qpart->verticalScrollBar()->value();
        QCOMPARE(sbBefore, 0);

        // Find thumb rect by asking minimap (we need to make viewportRect public for test)
        // For now, we will directly use the minimap's internal viewportRect via a friend hack
        // We can compute expected thumb position: for 10k lines, viewportLines ~ 40, thumb height ~ 120, at top y=0
        // So the thumb is from y=0 to y=120
        // Press at the center of the thumb (y=60)
        auto thumbH = 40 * 3; // viewportLines * lineHeight = 40*3=120
        auto pressY = thumbH / 2; // 60, inside thumb
        auto pressX = w / 2;
        auto pressPos = QPoint(pressX, pressY);
        qDebug() << "press inside thumb at" << pressPos << "thumbH" << thumbH;

        // Press inside thumb
        QTest::mousePress(mini, Qt::LeftButton, Qt::NoModifier, pressPos);
        QApplication::processEvents();

        // After press inside, the viewport should not jump much (still at 0, since we pressed inside)
        auto sbAfterPress = qpart->verticalScrollBar()->value();
        qDebug() << "sb after press inside" << sbAfterPress;
        // For inside press, the first line should still be near 0 (not centered)
        QVERIFY(qAbs(sbAfterPress - 0) < 50);

        // Now drag the thumb down by 200px
        auto dragDelta = 200;
        auto newY = pressY + dragDelta;
        auto newPos = QPoint(pressX, newY);
        // Calculate expected first line after drag:
        // For inside drag, target = (newPos.y() - dragOffset) * maxStart / maxViewportY
        auto dragOffset = pressY; // since thumb top was 0
        auto visibleH = h;
        auto total = 10000;
        auto viewportLines = qpart->viewport()->height() / qpart->fontMetrics().height();
        auto viewportHeight = viewportLines * 3;
        auto maxStart = total - viewportLines;
        auto maxViewportY = visibleH - viewportHeight;
        auto expectedFirstLine = 0;
        if (maxViewportY > 0) {
            expectedFirstLine = qRound(double(newY - dragOffset) * maxStart / maxViewportY);
        }
        qDebug() << "drag to" << newPos << "expected first line" << expectedFirstLine
                 << "viewportLines" << viewportLines << "viewportHeight" << viewportHeight
                 << "maxStart" << maxStart << "maxViewportY" << maxViewportY;

        QTest::mouseMove(mini, newPos);
        QApplication::processEvents();
        QTest::mouseRelease(mini, Qt::LeftButton, Qt::NoModifier, newPos);
        QApplication::processEvents();

        auto sbAfterDrag = qpart->verticalScrollBar()->value();
        qDebug() << "sb after drag" << sbAfterDrag << "expected" << expectedFirstLine;
        auto tolerance = 100;
        QVERIFY2(qAbs(sbAfterDrag - expectedFirstLine) < tolerance,
                 qPrintable(QString("sbAfterDrag %1 expected %2 tolerance %3 dragOffset %4 newY %5")
                                .arg(sbAfterDrag).arg(expectedFirstLine).arg(tolerance).arg(dragOffset).arg(newY)));

        // Also verify that the thumb now is at newPos - dragOffset
        // The new thumb top should be at newPos.y() - dragOffset = 200
        // Which corresponds to first line 200 * 10000 / 800 = 2500, matches expected

        delete qpart;
    }

    void testDragOutsideThumbIsProportional() {
        auto *qpart = new Qutepart::Qutepart();
        qpart->resize(1200, 800);
        qpart->show();
        QVERIFY(QTest::qWaitForWindowExposed(qpart));

        auto text = QString();
        for (auto i = 0; i < 10000; ++i) {
            text += QString("line %1\n").arg(i);
        }
        qpart->setPlainText(text);
        qpart->setMinimapVisible(true);
        QApplication::processEvents();

        auto *mini = qpart->findChild<Qutepart::Minimap*>();
        QVERIFY(mini);
        mini->resize(150, 800);
        QApplication::processEvents();

        auto h = mini->height();
        auto w = mini->width();

        // Click at 70% of minimap (outside thumb, since thumb at top)
        auto y70 = int(h * 0.70);
        auto pos70 = QPoint(w/2, y70);
        QTest::mousePress(mini, Qt::LeftButton, Qt::NoModifier, pos70);
        QTest::mouseRelease(mini, Qt::LeftButton, Qt::NoModifier, pos70);
        QApplication::processEvents();

        auto sb70 = qpart->verticalScrollBar()->value();
        auto expected70 = qRound(double(y70) * 10000 / h);
        qDebug() << "click at 70% y" << y70 << "sb" << sb70 << "expected" << expected70;
        auto tol = 100;
        QVERIFY2(qAbs(sb70 - expected70) < tol,
                 qPrintable(QString("70% click sb %1 expected %2").arg(sb70).arg(expected70)));

        delete qpart;
    }
};

QTEST_MAIN(TestMinimapDrag)
#include "test_minimap_drag.moc"

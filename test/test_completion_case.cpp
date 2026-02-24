#include <QDebug>
#include <QObject>
#include <QTest>
#include <QTimer>
#include <QListView>
#include <QKeyEvent>
#include <QCoreApplication>

#include "qutepart.h"

class TestCompletionCase : public QObject {
    Q_OBJECT

  private slots:
    void CompletionTabHonorsCaseSingleItem() {
        Qutepart::Qutepart qpart(nullptr, "");
        qpart.setCompletionEnabled(true);
        qpart.setCompletionThreshold(1);
        
        qpart.setPlainText("APPLE\napp");
        qpart.goTo(1, 3); // After "app"
        
        // Wait for word set update
        QTest::qWait(200);
        
        // Trigger completion manually
        qpart.invokeCompletionAction()->trigger();
        
        // Find the list widget
        QListView *list = nullptr;
        for (int i = 0; i < 10; ++i) {
            list = qpart.viewport()->findChild<QListView *>();
            if (list) break;
            QTest::qWait(100);
        }
        
        QVERIFY(list != nullptr);
        
        // Simulate Tab press
        QKeyEvent tabEvent(QEvent::KeyPress, Qt::Key_Tab, Qt::NoModifier);
        QCoreApplication::sendEvent(qpart.viewport(), &tabEvent);
        
        // Expected: APPLE
        // Actual (per bug report): apple
        QCOMPARE(qpart.lines().at(1).text(), QString("APPLE"));
    }

    void CompletionTabHonorsCaseMultipleItems() {
        Qutepart::Qutepart qpart(nullptr, "");
        qpart.setCompletionEnabled(true);
        qpart.setCompletionThreshold(1);
        
        qpart.setPlainText("Apple Application\napp");
        qpart.goTo(1, 3); // After "app"
        
        // Wait for word set update
        QTest::qWait(200);
        
        // Trigger completion manually
        qpart.invokeCompletionAction()->trigger();
        
        // Find the list widget
        QListView *list = nullptr;
        for (int i = 0; i < 10; ++i) {
            list = qpart.viewport()->findChild<QListView *>();
            if (list) break;
            QTest::qWait(100);
        }
        
        QVERIFY(list != nullptr);
        
        // Simulate Tab press
        QKeyEvent tabEvent(QEvent::KeyPress, Qt::Key_Tab, Qt::NoModifier);
        QCoreApplication::sendEvent(qpart.viewport(), &tabEvent);
        
        // Expected: Appl (common prefix of Apple and Application is Appl)
        // Actual (per theory): appl
        QCOMPARE(qpart.lines().at(1).text(), QString("Appl"));
    }
};

QTEST_MAIN(TestCompletionCase)
#include "test_completion_case.moc"

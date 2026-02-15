/*
 * Copyright (C) 2023-...  Diego Iastrubni <diegoiast@gmail.com>
 * SPDX-License-Identifier: MIT
 */

#include <QDebug>
#include <QObject>
#include <QTest>
#include <QTimer>
#include <QListView>

#include "qutepart.h"

class Test : public QObject {
    Q_OBJECT

  private slots:
    void CompletionReplacePrefix() {
        Qutepart::Qutepart qpart(nullptr, "");
        qpart.setCompletionEnabled(true);
        qpart.setCompletionThreshold(1);
        
        qpart.setPlainText("IMPLemented\nimpl");
        qpart.goTo(1, 4);
        
        // Wait for word set update
        QTest::qWait(200);
        
        // Trigger completion manually
        qpart.invokeCompletionAction()->trigger();
        
        // Use a loop to find the list widget
        QListView *list = nullptr;
        for (int i = 0; i < 10; ++i) {
            list = qpart.viewport()->findChild<QListView *>();
            if (list) break;
            QTest::qWait(100);
        }
        
        QVERIFY(list != nullptr);
        
        QMetaObject::invokeMethod(list, "itemSelected", Q_ARG(int, 0));
        
        QTRY_COMPARE(qpart.lines().at(1).text(), QString("IMPLemented"));
    }

    void CompletionReplaceWholeWord() {
        Qutepart::Qutepart qpart(nullptr, "");
        qpart.setPlainText("MY_IMPLEMENTATION\nmy_impl_test");
        qpart.setCompletionEnabled(true);
        qpart.setCompletionThreshold(1);
        
        qpart.goTo(1, 7); // After "my_impl"
        
        // Wait for word set update
        QTest::qWait(200);
        
        qpart.invokeCompletionAction()->trigger();
        
        QListView *list = nullptr;
        for (int i = 0; i < 10; ++i) {
            list = qpart.viewport()->findChild<QListView *>();
            if (list) break;
            QTest::qWait(100);
        }
        
        QVERIFY(list != nullptr);
        
        QMetaObject::invokeMethod(list, "itemSelected", Q_ARG(int, 0));
        
        QTRY_COMPARE(qpart.lines().at(1).text(), QString("MY_IMPLEMENTATION"));
    }
};

QTEST_MAIN(Test)
#include "test_completion.moc"

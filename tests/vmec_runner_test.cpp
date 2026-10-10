#include "jobs/vmec_runner.h"

#include <QCoreApplication>
#include <QEventLoop>
#include <QTemporaryDir>
#include <QTimer>

#include <gtest/gtest.h>

using namespace astron::jobs;

TEST(VmecRunnerTest, RejectsMissingInput)
{
    VmecRunner runner;
    const VmecRunRequest request{{}, {}, QCoreApplication::applicationFilePath(), 1};

    const auto result = runner.start(request);

    EXPECT_FALSE(result.succeeded());
    EXPECT_FALSE(result.error.isEmpty());
    EXPECT_FALSE(runner.isRunning());
}

TEST(VmecRunnerTest, CancelsRunningProcessWithoutImportingResult)
{
    QTemporaryDir runsRoot;
    ASSERT_TRUE(runsRoot.isValid());
    VmecRunner runner(nullptr, runsRoot.path());
    const VmecRunRequest request{"--solver-child", "wait for cancellation", QCoreApplication::applicationFilePath(), 1};
    QEventLoop loop;
    QEventLoop readyLoop;
    bool completed = false;
    bool cancelled = false;
    bool ready = false;
    QString resultPath;
    QObject::connect(&runner, &VmecRunner::logMessage, &readyLoop,
                     [&](const QString& message)
                     {
                         if (message.contains("READY"))
                         {
                             ready = true;
                             readyLoop.quit();
                         }
                     });
    QObject::connect(&runner, &VmecRunner::finished, &loop,
                     [&](const bool wasCancelled, const QString& path, const QString&)
                     {
                         completed = true;
                         cancelled = wasCancelled;
                         resultPath = path;
                         loop.quit();
                     });

    const auto started = runner.start(request);
    ASSERT_TRUE(started.succeeded()) << started.error.toStdString();
    EXPECT_TRUE(runner.isRunning());
    QTimer::singleShot(3000, &readyLoop, &QEventLoop::quit);
    readyLoop.exec();
    ASSERT_TRUE(ready);
    runner.cancel();
    QTimer::singleShot(10000, &loop, &QEventLoop::quit);
    loop.exec();

    EXPECT_TRUE(completed);
    EXPECT_TRUE(cancelled);
    EXPECT_TRUE(resultPath.isEmpty());
    EXPECT_FALSE(runner.isRunning());
}

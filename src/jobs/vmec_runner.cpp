#include "jobs/vmec_runner.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QProcessEnvironment>
#include <QStandardPaths>
#include <QUuid>

#include <algorithm>
#include <utility>

using namespace astron::jobs;

VmecRunner::VmecRunner(QObject* parent, QString runsRoot) : QObject(parent), runsRoot_(std::move(runsRoot))
{
    killTimer_.setSingleShot(true);
    killTimer_.setInterval(5000);
    connect(&killTimer_, &QTimer::timeout, this,
            [this]()
            {
                if(active_ && process_.state() != QProcess::NotRunning)
                {
                    emit logMessage("VMEC++ did not stop in five seconds; killing the process.");
                    process_.kill();
                }
            });
    connect(&process_, &QProcess::started, this,
            [this]()
            {
                if(cancelRequested_)
                {
                    process_.terminate();
                }
            });
    connect(&process_, &QProcess::readyReadStandardOutput, this, [this]() { emit logMessage(QString::fromUtf8(process_.readAllStandardOutput())); });
    connect(&process_, &QProcess::readyReadStandardError, this, [this]() { emit logMessage(QString::fromUtf8(process_.readAllStandardError())); });
    connect(&process_, &QProcess::errorOccurred, this, [this](const QProcess::ProcessError error) { handleError(error); });
    connect(&process_, &QProcess::finished, this, [this](const int code, const QProcess::ExitStatus status) { finishProcess(code, status); });
}

bool VmecRunner::isRunning() const
{
    return active_;
}

VmecStartResult VmecRunner::start(const VmecRunRequest& request)
{
    if(active_)
    {
        return {{}, QStringLiteral("VMEC++ is already running.")};
    }
    if(request.inputPath.isEmpty() || request.inputText.isEmpty())
    {
        return {{}, QStringLiteral("Open a VMEC input file first.")};
    }
    if(request.executable.trimmed().isEmpty())
    {
        return {{}, QStringLiteral("Set the VMEC++ executable path.")};
    }

    const QFileInfo original(request.inputPath);
    const QString base = runsRoot_.isEmpty() ? QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/runs" : runsRoot_;
    const QString runDirectory = base + "/" + QUuid::createUuid().toString(QUuid::WithoutBraces);
    if(!QDir().mkpath(runDirectory))
    {
        return {{}, QStringLiteral("Cannot create the run directory.")};
    }

    QFile input(runDirectory + "/" + original.fileName());
    if(!input.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        return {{}, input.errorString()};
    }
    const QByteArray contents = request.inputText.toUtf8();
    if(input.write(contents) != contents.size() || !input.flush())
    {
        return {{}, input.errorString()};
    }
    input.close();

    auto environment = QProcessEnvironment::systemEnvironment();
    environment.insert("OMP_NUM_THREADS", QString::number(std::max(1, request.threads)));
    environment.insert("OPENBLAS_NUM_THREADS", "1");
    environment.insert("MKL_NUM_THREADS", "1");
    process_.setWorkingDirectory(runDirectory);
    process_.setProcessEnvironment(environment);
    activeRunDirectory_ = runDirectory;
    cancelRequested_ = false;
    active_ = true;
    process_.start(request.executable.trimmed(), {original.fileName()});
    return {runDirectory, {}};
}

void VmecRunner::cancel()
{
    if(!active_ || cancelRequested_)
    {
        return;
    }
    cancelRequested_ = true;
    emit logMessage("Stopping VMEC++...");
    if(process_.state() == QProcess::Running)
    {
        process_.terminate();
    }
    killTimer_.start();
}

void VmecRunner::handleError(const QProcess::ProcessError error)
{
    if(!active_)
    {
        return;
    }
    if(error == QProcess::FailedToStart)
    {
        killTimer_.stop();
        active_ = false;
        emit finished(cancelRequested_, {}, cancelRequested_ ? QStringLiteral("VMEC++ run stopped.") : QStringLiteral("VMEC++ could not start; check its executable path."));
    }
    else
    {
        emit logMessage(QString("VMEC++ process error: %1").arg(process_.errorString()));
    }
}

void VmecRunner::finishProcess(const int exitCode, const QProcess::ExitStatus status)
{
    if(!active_)
    {
        return;
    }
    killTimer_.stop();
    active_ = false;
    emit logMessage(QString("VMEC++ finished with exit code %1").arg(exitCode));
    if(cancelRequested_)
    {
        emit finished(true, {}, QStringLiteral("VMEC++ run stopped."));
        return;
    }
    if(status != QProcess::NormalExit || exitCode != 0)
    {
        emit finished(false, {}, QStringLiteral("VMEC++ failed; inspect the run log."));
        return;
    }
    const QStringList outputs = QDir(activeRunDirectory_).entryList({"wout_*.nc"}, QDir::Files);
    if(outputs.isEmpty())
    {
        emit finished(false, {}, QStringLiteral("VMEC++ finished, but no wout_*.nc was found in the run directory."));
        return;
    }
    emit finished(false, activeRunDirectory_ + "/" + outputs.first(), QStringLiteral("VMEC++ equilibrium complete."));
}

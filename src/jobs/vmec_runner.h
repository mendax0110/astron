#pragma once

#include <QObject>
#include <QProcess>
#include <QString>
#include <QTimer>

/**
 * @brief Astron namespace for job management \namespace astron::jobs
 */
namespace astron::jobs
{
    /**
     * @brief Represents the result of starting a VMEC run. \struct VmecStartResult
     */
    struct VmecStartResult
    {
        QString runDirectory;
        QString error;

        /**
         * @brief Checks if the VMEC run was started successfully.
         * @return true if the run was started successfully (runDirectory is not empty), false otherwise.
         */
        [[nodiscard]] bool succeeded() const
        {
            return !runDirectory.isEmpty();
        }
    };

    /**
     * @brief Represents a request to run VMEC. \struct VmecRunRequest
     */
    struct VmecRunRequest
    {
        QString inputPath;
        QString inputText;
        QString executable;
        int threads = 1;
    };

    /**
     * @brief Manages the execution of VMEC runs in a separate process. \class VmecRunner
     * @details Inherits from QObject to utilize Qt's signal-slot mechanism for asynchronous communication.
     */
    class VmecRunner final : public QObject
    {
        Q_OBJECT

    public:
        /**
         * @brief Constructs a VmecRunner object.
         * @param parent The parent QObject, defaulting to nullptr.
         * @param runsRoot The root directory for VMEC runs, defaulting to an empty string.
         */
        explicit VmecRunner(QObject* parent = nullptr, QString runsRoot = {});

        /**
         * @brief Checks if a VMEC run is currently active.
         * @return true if a run is active, false otherwise.
         */
        [[nodiscard]] bool isRunning() const;

        /**
         * @brief Starts a VMEC run with the specified request parameters.
         * @param request The VmecRunRequest containing input path, input text, executable path, and thread count.
         * @return A VmecStartResult indicating the success or failure of starting the run.
         */
        [[nodiscard]] VmecStartResult start(const VmecRunRequest& request);

        /**
         * @brief Cancels the currently active VMEC run, if any.
         */
        void cancel();

    signals:

        /**
         * @brief Signal emitted when a log message is generated during the VMEC run.
         * @param message The log message as a QString.
         */
        void logMessage(const QString& message);

        /**
         * @brief Signal emitted when the VMEC run has finished, either successfully or due to cancellation.
         * @param cancelled A boolean indicating whether the run was cancelled.
         * @param resultPath The path to the result file, if available.
         * @param message A message describing the outcome of the run.
         */
        void finished(bool cancelled, const QString& resultPath, const QString& message);

    private:

        /**
         * @brief Handles the completion of the VMEC process, emitting the finished signal with appropriate parameters.
         * @param exitCode The exit code of the process.
         * @param status The exit status of the process (normal or crash).
         */
        void finishProcess(int exitCode, QProcess::ExitStatus status);

        /**
         * @brief Handles errors that occur during the execution of the VMEC process, emitting log messages and finished signals as needed.
         * @param error The type of process error that occurred.
         */
        void handleError(QProcess::ProcessError error);

        QProcess process_;
        QTimer killTimer_;
        QString activeRunDirectory_;
        QString runsRoot_;
        bool active_ = false;
        bool cancelRequested_ = false;
    };
} // namespace astron::jobs

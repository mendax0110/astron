#pragma once

#include "core/equilibrium.h"
#include "jobs/vmec_runner.h"
#include "simulation/particle.h"

#include <QMainWindow>
#include <QString>
#include <QTimer>

#include <vtkSmartPointer.h>

#include <array>
#include <cstddef>
#include <optional>
#include <vector>

class QCheckBox;
class QDoubleSpinBox;
class QLabel;
class QLineEdit;
class QPlainTextEdit;
class QPushButton;
class QSlider;
class QSpinBox;
class QVTKOpenGLNativeWidget;
class vtkActor;
class vtkGenericOpenGLRenderWindow;
class vtkRenderer;

/**
 * @brief Astron namespace for the main application window \namespace astron
 */
namespace astron
{
    /**
     * @brief Represents the main application window for Astron. \class MainWindow
     * @details Inherits from QMainWindow to provide a graphical user interface for interacting with VMEC results and visualizations.
     */
    class MainWindow final : public QMainWindow
    {
    public:
        /**
         * @brief Constructs a MainWindow object.
         * @param parent The parent QWidget, defaulting to nullptr.
         */
        explicit MainWindow(QWidget* parent = nullptr);

    private:
        /**
         * @brief Builds the user interface components of the main window.
         */
        void buildUi();

        /**
         * @brief Builds the 3D scene for visualization using VTK.
         */
        void buildScene();

        /**
         * @brief Opens a VMEC result file and loads the equilibrium data.
         */
        void openResult();

        /**
         * @brief Opens a VMEC input file and loads the input text.
         */
        void openInput();

        /**
         * @brief Reads the input controls from the user interface and updates the internal state.
         */
        void readInputControls();

        /**
         * @brief Applies the input controls from the user interface to the internal state.
         */
        void applyInputControls();

        /**
         * @brief Loads a VMEC result from the specified path and updates the visualization.
         * @param path The path to the VMEC result file.
         */
        void loadResult(const QString& path);

        /**
         * @brief Starts the VMEC run using the current input and settings.
         */
        void runVmec();

        /**
         * @brief Finishes the VMEC run, updating the UI and loading the result if successful.
         * @param cancelled Indicates whether the run was cancelled.
         * @param resultPath The path to the VMEC result file, if available.
         * @param message A message describing the outcome of the run.
         */
        void finishVmec(bool cancelled, const QString& resultPath, const QString& message);

        /**
         * @brief Rebuilds the surface visualization based on the current equilibrium and slider value.
         */
        void rebuildSurface() const;

        /**
         * @brief Rebuilds the guide path visualization based on the current equilibrium and slider value.
         */
        void rebuildGuide();

        /**
         * @brief Updates the position of the marker along the guide path for animation.
         */
        void updateMarker();

        /**
         * @brief Updates the metrics display based on the current equilibrium and slider value.
         */
        void updateMetrics() const;

        /**
         * @brief Appends a log message to the log display in the user interface.
         * @param message The log message to append.
         */
        void appendLog(const QString& message) const;

        /**
         * @brief Starts the particle simulation, initializing particles and starting the simulation timer.
         */
        void startParticleSimulation();

        /**
         * @brief Stops the particle simulation, stopping the simulation timer and clearing particles.
         */
        void stopParticleSimulation();

        /**
         * @brief Advances the particle simulation by one time step, updating particle positions and velocities.
         */
        void particleStep();

        QVTKOpenGLNativeWidget* viewport_ = nullptr;
        QLabel* sourceLabel_ = nullptr;
        QLabel* metricsLabel_ = nullptr;
        QSlider* surfaceSlider_ = nullptr;
        QCheckBox* coilsCheck_ = nullptr;
        QCheckBox* guideCheck_ = nullptr;
        QCheckBox* animateCheck_ = nullptr;
        QLineEdit* inputPath_ = nullptr;
        QLineEdit* solverPath_ = nullptr;
        QPlainTextEdit* inputEditor_ = nullptr;
        QDoubleSpinBox* fluxSpin_ = nullptr;
        QDoubleSpinBox* currentSpin_ = nullptr;
        QSpinBox* poloidalModesSpin_ = nullptr;
        QSpinBox* toroidalModesSpin_ = nullptr;
        QSpinBox* solverThreadsSpin_ = nullptr;
        std::array<bool, 4> inputDirty_ = {};
        QPlainTextEdit* log_ = nullptr;
        QPushButton* runButton_ = nullptr;
        QPushButton* stopButton_ = nullptr;

        vtkSmartPointer<vtkGenericOpenGLRenderWindow> renderWindow_;
        vtkSmartPointer<vtkRenderer> renderer_;
        vtkSmartPointer<vtkActor> plasmaActor_;
        vtkSmartPointer<vtkActor> coilsActor_;
        vtkSmartPointer<vtkActor> guideActor_;
        vtkSmartPointer<vtkActor> markerActor_;

        std::optional<core::Equilibrium> equilibrium_;
        std::vector<std::array<double, 3>> guidePath_;
        size_t markerIndex_ = 0;
        jobs::VmecRunner runner_;
        QTimer animationTimer_;

        QPushButton* startParticlesButton_ = nullptr;
        QPushButton* stopParticlesButton_ = nullptr;
        QCheckBox* relativisticCheck_ = nullptr;
        QTimer particleTimer_;
        std::vector<simulation::Particle> simParticles_;
        std::vector<vtkSmartPointer<vtkActor>> simParticleActors_;
    };
} // namespace astron

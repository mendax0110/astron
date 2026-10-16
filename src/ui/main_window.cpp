#include "ui/main_window.h"
#include "io/vmec_netcdf_reader.h"
#include "visualization/stellarator_geometry.h"

#include <QCheckBox>
#include <QDir>
#include <QDockWidget>
#include <QDoubleSpinBox>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QFormLayout>
#include <QGroupBox>
#include <QGuiApplication>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QRegularExpression>
#include <QScrollArea>
#include <QSlider>
#include <QSpinBox>
#include <QStatusBar>
#include <QTextDocument>
#include <QThread>
#include <QVBoxLayout>
#include <QVTKOpenGLNativeWidget.h>
#include <QApplication>

#include "simulation/boris_integrator.h"
#include "simulation/constants.h"
#include "simulation/field_provider.h"
#include "simulation/particle.h"
#include "simulation/vmec_field_provider.h"

#include <vtkActor.h>
#include <vtkCellArray.h>
#include <vtkGenericOpenGLRenderWindow.h>
#include <vtkPoints.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkRenderer.h>
#include <vtkSphereSource.h>

#include <algorithm>
#include <cmath>
#include <exception>
#include <memory>
#include <numbers>
#include <random>
#include <utility>

using namespace astron;

namespace
{
    constexpr int ParticleCount = 16;
    constexpr int ParticleSubstepsPerFrame = 100;
    constexpr double ParticleEnergyEv = 1000.0;
    constexpr double StepsPerGyroperiod = 40.0;
    constexpr double StartRhoMin = 0.2;
    constexpr double StartRhoMax = 0.8;

    /**
     * @brief Returns a QString containing the scientific style sheet for the application.
     * @return A QString containing the scientific style sheet.
     */
    QString scientificStyleSheet()
    {
        return R"(
           QMainWindow, QWidget, QDockWidget, QGroupBox, QAbstractScrollArea, QStatusBar, QLabel, QPlainTextEdit, QToolTip {
               background-color: #10171d;
               color: #edf4fe;
           }
           QDockWidget {
               border: 1px solid #2d3944;
           }
           QDockWidget::title {
               background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #1a2731, stop:1 #121b24);
               color: #edf4fe;
               padding: 8px 12px;
               font-weight: 600;
               border-bottom: 1px solid #2d3944;
           }
           QGroupBox {
               border: 1px solid #2d3944;
               border-radius: 6px;
               margin-top: 10px;
               padding-top: 12px;
           }
           QGroupBox::title {
               subcontrol-origin: margin;
               left: 12px;
               color: #dfeeff;
           }
           QPushButton {
               background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #1c2c38, stop:1 #16232d);
               border: 1px solid #49657a;
               border-radius: 5px;
               padding: 6px 12px;
               color: #edf4fe;
               font-weight: 500;
           }
           QPushButton:hover {
               background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #1f3748, stop:1 #1a2d39);
               border-color: #5f86a0;
           }
           QPushButton:pressed {
               background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #162833, stop:1 #101b24);
           }
           QPushButton:disabled {
               background: #1a2129;
               color: #8899ab;
               border-color: #2b3947;
           }
           QLineEdit, QPlainTextEdit, QSpinBox, QDoubleSpinBox {
               background-color: #0d141b;
               border: 1px solid #324a5d;
               border-radius: 4px;
               color: #edf4fe;
               padding: 4px 6px;
               selection-background-color: #2d6d8c;
           }
           QCheckBox {
               color: #edf4fe;
           }
           QCheckBox::indicator {
               width: 14px;
               height: 14px;
               background-color: #0d141b;
               border: 1px solid #405b71;
               border-radius: 3px;
           }
           QCheckBox::indicator:checked {
               background-color: #5ea6d9;
               border-color: #5ea6d9;
           }
           QSlider::groove:horizontal {
               background: #1b2630;
               border: 1px solid #2f3c49;
               height: 6px;
               border-radius: 3px;
           }
           QSlider::handle:horizontal {
               background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #8fd9b4, stop:1 #5ea6d9);
               border: 1px solid #dfeeff;
               width: 14px;
               margin: -4px 0;
               border-radius: 7px;
           }
           QSlider::sub-page:horizontal {
               background: #285067;
               border-radius: 3px;
           }
           QLabel {
               color: #edf4fe;
           }
           QStatusBar {
               background: #10171d;
               color: #dfeeff;
               border-top: 1px solid #2d3944;
           }
           QToolTip {
               background-color: #1a2731;
               color: #edf4fe;
               border: 1px solid #44708f;
               padding: 6px 8px;
           }
       )";
    }

    /**
     * @brief Finds the input parameter in the source text using a regular expression.
     * @param source The source text to search.
     * @param name The name of the input parameter to find.
     * @return A QRegularExpressionMatch containing the match information, or an empty match if not found.
     */
    QRegularExpressionMatch findInputParameter(const QString& source, const QString& name)
    {
        const QRegularExpression pattern(QStringLiteral("\\b%1\\s*=\\s*([+-]?(?:\\d+(?:\\.\\d*)?|\\.\\d+)(?:[eEdD][+-]?\\d+)?)").arg(QRegularExpression::escape(name)),
                                         QRegularExpression::CaseInsensitiveOption);
        auto matches = pattern.globalMatch(source);
        while (matches.hasNext())
        {
            const auto match = matches.next();
            const qsizetype lineStart = source.lastIndexOf(QLatin1Char('\n'), match.capturedStart()) + 1;
            const QString prefix = source.mid(lineStart, match.capturedStart() - lineStart);
            if (!prefix.contains(QLatin1Char('!')) && !prefix.contains(QLatin1Char('#')))
            {
                return match;
            }
        }
        return {};
    }

    /**
     * @brief Reads the value of an input parameter from the source text.
     * @param source The source text to search.
     * @param name The name of the input parameter to read.
     * @return An optional double containing the value of the input parameter, or std::nullopt if not found or invalid.
     */
    std::optional<double> readInputParameter(const QString& source, const QString& name)
    {
        const auto match = findInputParameter(source, name);
        if (!match.hasMatch())
        {
            return std::nullopt;
        }
        QString number = match.captured(1);
        number.replace(QLatin1Char('D'), QLatin1Char('E'), Qt::CaseInsensitive);
        bool valid = false;
        const double value = number.toDouble(&valid);
        if (!valid || !std::isfinite(value))
        {
            return std::nullopt;
        }
        return value;
    }

    /**
     * @brief Replaces the value of an input parameter in the source text.
     * @param source The source text to modify.
     * @param name The name of the input parameter to replace.
     * @param value The new value for the input parameter.
     * @return True if the parameter was found and replaced, false otherwise.
     */
    bool replaceInputParameter(QString& source, const QString& name, const QString& value)
    {
        const auto match = findInputParameter(source, name);
        if (!match.hasMatch())
        {
            return false;
        }
        source.replace(match.capturedStart(1), match.capturedLength(1), value);
        return true;
    }

    /**
     * @brief A RAII class that sets the cursor to a wait cursor when constructed and restores the previous cursor when destroyed. \class WaitCursor
     */
    class WaitCursor
    {
    public:
        /**
         * @brief Constructs a WaitCursor object and sets the cursor to a wait cursor.
         */
        WaitCursor()
        {
            QApplication::setOverrideCursor(Qt::WaitCursor);
        }

        /**
         * @brief Destructor that restores the previous cursor.
         */
        ~WaitCursor()
        {
            QApplication::restoreOverrideCursor();
        }

        WaitCursor(const WaitCursor&) = delete;
        WaitCursor& operator=(const WaitCursor&) = delete;
    };

} // namespace

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent)
{
    try
    {
        buildUi();
        buildScene();
        rebuildSurface();
        rebuildGuide();
        updateMetrics();
        renderer_->ResetCamera();
        renderWindow_->Render();

        animationTimer_.setInterval(100);
        connect(&animationTimer_, &QTimer::timeout, this, [this]() { updateMarker(); });
        particleTimer_.setInterval(16); // about 60 Hz
        connect(&particleTimer_, &QTimer::timeout, this, [this]() { particleStep(); });
        connect(&runner_, &jobs::VmecRunner::logMessage, this, [this](const QString& message) { appendLog(message); });
        connect(&runner_, &jobs::VmecRunner::finished, this,
                [this](const bool cancelled, const QString& resultPath, const QString& message) { finishVmec(cancelled, resultPath, message); });
    }
    catch (const std::exception& e)
    {
        QMessageBox::critical(this, "Error", QString("Failed to initialize the application: %1").arg(e.what()));
        throw;
    }
}

MainWindow::~MainWindow()
{
    runner_.disconnect(this);
    runner_.cancel();
}

void MainWindow::buildUi()
{
    setWindowTitle("Astron Wendelstein 7-X workbench");
    resize(1450, 900);
    setStyleSheet(scientificStyleSheet());

    viewport_ = new QVTKOpenGLNativeWidget(this);
    setCentralWidget(viewport_);

    auto* controlsDock = new QDockWidget("Scenario and display", this);
    controlsDock->setMinimumWidth(300);
    auto* controlsScroll = new QScrollArea(controlsDock);
    controlsScroll->setWidgetResizable(true);
    auto* controls = new QWidget(controlsScroll);
    auto* controlsLayout = new QVBoxLayout(controls);
    sourceLabel_ = new QLabel("SCHEMATIC W7-X VIEW\nNo equilibrium loaded", controls);
    sourceLabel_->setWordWrap(true);
    controlsLayout->addWidget(sourceLabel_);
    auto* openResultButton = new QPushButton("Open VMEC result (.nc)", controls);
    controlsLayout->addWidget(openResultButton);
    controlsLayout->addWidget(new QLabel("Selected flux surface (release slider to update)", controls));
    surfaceSlider_ = new QSlider(Qt::Horizontal, controls);
    surfaceSlider_->setRange(5, 100);
    surfaceSlider_->setValue(100);
    surfaceSlider_->setTracking(false);
    controlsLayout->addWidget(surfaceSlider_);
    coilsCheck_ = new QCheckBox("Show schematic coils", controls);
    coilsCheck_->setChecked(true);
    controlsLayout->addWidget(coilsCheck_);
    guideCheck_ = new QCheckBox("Show illustrative surface path", controls);
    guideCheck_->setChecked(true);
    controlsLayout->addWidget(guideCheck_);
    animateCheck_ = new QCheckBox("Animate illustrative marker", controls);
    animateCheck_->setChecked(false);
    animateCheck_->setToolTip("Animation redraws the full 3D scene. Leave it off when exploring a large equilibrium or using software rendering.");
    controlsLayout->addWidget(animateCheck_);
    controlsLayout->addSpacing(22);
    controlsLayout->addWidget(new QLabel("VMEC++ input file", controls));
    inputPath_ = new QLineEdit(controls);
    inputPath_->setPlaceholderText("Select input.w7x");
    controlsLayout->addWidget(inputPath_);
    auto* openInputButton = new QPushButton("Open input", controls);
    controlsLayout->addWidget(openInputButton);
    auto* parametersGroup = new QGroupBox("VMEC input parameters", controls);
    auto* parametersLayout = new QFormLayout(parametersGroup);
    fluxSpin_ = new QDoubleSpinBox(parametersGroup);
    fluxSpin_->setRange(-100000.0, 100000.0);
    fluxSpin_->setDecimals(12);
    fluxSpin_->setSingleStep(0.1);
    fluxSpin_->setSuffix(" Wb");
    parametersLayout->addRow("Edge flux (PHIEDGE)", fluxSpin_);
    currentSpin_ = new QDoubleSpinBox(parametersGroup);
    currentSpin_->setRange(-1000000000.0, 1000000000.0);
    currentSpin_->setDecimals(12);
    currentSpin_->setSingleStep(1000.0);
    currentSpin_->setSuffix(" A");
    parametersLayout->addRow("Toroidal current (CURTOR)", currentSpin_);
    poloidalModesSpin_ = new QSpinBox(parametersGroup);
    poloidalModesSpin_->setRange(1, 256);
    parametersLayout->addRow("Poloidal modes (MPOL)", poloidalModesSpin_);
    toroidalModesSpin_ = new QSpinBox(parametersGroup);
    toroidalModesSpin_->setRange(0, 256);
    parametersLayout->addRow("Toroidal modes (NTOR)", toroidalModesSpin_);
    for (QWidget* control : std::array<QWidget*, 4>{fluxSpin_, currentSpin_, poloidalModesSpin_, toroidalModesSpin_})
    {
        control->setEnabled(false);
    }
    controlsLayout->addWidget(parametersGroup);
    auto* readControlsButton = new QPushButton("Read values from editor", controls);
    controlsLayout->addWidget(readControlsButton);
    auto* applyControlsButton = new QPushButton("Apply values to editor", controls);
    controlsLayout->addWidget(applyControlsButton);
    controlsLayout->addWidget(new QLabel("Solver executable", controls));
    const QString localSolver = QDir::homePath() + "/.local/bin/vmecpp";
    solverPath_ = new QLineEdit(QFileInfo::exists(localSolver) ? localSolver : "vmecpp", controls);
    controlsLayout->addWidget(solverPath_);
    solverThreadsSpin_ = new QSpinBox(controls);
    const int availableThreads = std::max(1, QThread::idealThreadCount());
    solverThreadsSpin_->setRange(1, availableThreads);
    solverThreadsSpin_->setValue(std::min(4, std::max(1, availableThreads - 1)));
    solverThreadsSpin_->setToolTip("Limits VMEC++ OpenMP workers for new runs so the desktop keeps CPU time.");
    auto* solverThreadsLayout = new QFormLayout();
    solverThreadsLayout->addRow("Solver CPU threads", solverThreadsSpin_);
    controlsLayout->addLayout(solverThreadsLayout);
    runButton_ = new QPushButton("Run equilibrium", controls);
    controlsLayout->addWidget(runButton_);
    stopButton_ = new QPushButton("Stop VMEC++", controls);
    stopButton_->setEnabled(false);
    controlsLayout->addWidget(stopButton_);

    // Particle simulation controls
    startParticlesButton_ = new QPushButton("Start particles", controls);
    controlsLayout->addWidget(startParticlesButton_);
    stopParticlesButton_ = new QPushButton("Stop particles", controls);
    stopParticlesButton_->setEnabled(false);
    controlsLayout->addWidget(stopParticlesButton_);
    relativisticCheck_ = new QCheckBox("Relativistic Boris", controls);
    relativisticCheck_->setChecked(false);
    controlsLayout->addWidget(relativisticCheck_);

    controlsLayout->addStretch();
    controlsScroll->setWidget(controls);
    controlsDock->setWidget(controlsScroll);
    addDockWidget(Qt::LeftDockWidgetArea, controlsDock);

    auto* metricsDock = new QDockWidget("Measurements", this);
    auto* metricsPanel = new QWidget(metricsDock);
    auto* metricsLayout = new QVBoxLayout(metricsPanel);
    metricsLabel_ = new QLabel(metricsPanel);
    metricsLabel_->setWordWrap(true);
    metricsLabel_->setTextInteractionFlags(Qt::TextSelectableByMouse);
    metricsLayout->addWidget(metricsLabel_);
    metricsLayout->addStretch();
    metricsDock->setWidget(metricsPanel);
    addDockWidget(Qt::RightDockWidgetArea, metricsDock);

    auto* outputDock = new QDockWidget("VMEC input and run log", this);
    auto* output = new QWidget(outputDock);
    auto* outputLayout = new QVBoxLayout(output);
    outputLayout->addWidget(new QLabel("Edit the VMEC input here. Each run keeps its own copy.", output));
    inputEditor_ = new QPlainTextEdit(output);
    inputEditor_->setPlaceholderText("Open a VMEC input file to edit its parameters.");
    inputEditor_->setMinimumHeight(115);
    outputLayout->addWidget(inputEditor_);
    log_ = new QPlainTextEdit(output);
    log_->setReadOnly(true);
    log_->document()->setMaximumBlockCount(2000);
    log_->setMinimumHeight(80);
    outputLayout->addWidget(log_);
    outputDock->setWidget(output);
    addDockWidget(Qt::BottomDockWidgetArea, outputDock);
    statusBar()->showMessage("Open a VMEC result for calculated geometry.");

    connect(openResultButton, &QPushButton::clicked, this, [this]() { openResult(); });
    connect(openInputButton, &QPushButton::clicked, this, [this]() { openInput(); });
    connect(readControlsButton, &QPushButton::clicked, this, [this]() { readInputControls(); });
    connect(applyControlsButton, &QPushButton::clicked, this, [this]() { applyInputControls(); });
    connect(fluxSpin_, &QDoubleSpinBox::valueChanged, this, [this](double) { inputDirty_[0] = true; });
    connect(currentSpin_, &QDoubleSpinBox::valueChanged, this, [this](double) { inputDirty_[1] = true; });
    connect(poloidalModesSpin_, &QSpinBox::valueChanged, this, [this](int) { inputDirty_[2] = true; });
    connect(toroidalModesSpin_, &QSpinBox::valueChanged, this, [this](int) { inputDirty_[3] = true; });
    connect(runButton_, &QPushButton::clicked, this, [this]() { runVmec(); });
    connect(stopButton_, &QPushButton::clicked, this,
            [this]()
            {
                runner_.cancel();
                stopButton_->setEnabled(false);
                statusBar()->showMessage("Stopping VMEC++...");
            });

    connect(startParticlesButton_, &QPushButton::clicked, this,
            [this]()
            {
                if (startParticleSimulation())
                {
                    startParticlesButton_->setEnabled(false);
                    stopParticlesButton_->setEnabled(true);
                }
            });
    connect(stopParticlesButton_, &QPushButton::clicked, this,
            [this]()
            {
                stopParticlesButton_->setEnabled(false);
                startParticlesButton_->setEnabled(true);
                stopParticleSimulation();
            });
    connect(surfaceSlider_, &QSlider::valueChanged, this,
            [this]()
            {
                rebuildSurface();
                rebuildGuide();
                updateMetrics();
                renderWindow_->Render();
            });
    connect(animateCheck_, &QCheckBox::toggled, this,
            [this](const bool enabled)
            {
                if (enabled && guideCheck_->isChecked())
                {
                    animationTimer_.start();
                }
                else
                {
                    animationTimer_.stop();
                }
            });
    connect(coilsCheck_, &QCheckBox::toggled, this,
            [this](const bool visible)
            {
                coilsActor_->SetVisibility(visible);
                renderWindow_->Render();
            });
    connect(guideCheck_, &QCheckBox::toggled, this,
            [this](const bool visible)
            {
                guideActor_->SetVisibility(visible);
                markerActor_->SetVisibility(visible);
                if (visible && animateCheck_->isChecked())
                {
                    animationTimer_.start();
                }
                else
                {
                    animationTimer_.stop();
                }
                renderWindow_->Render();
            });
}

void MainWindow::buildScene()
{
    renderWindow_ = vtkSmartPointer<vtkGenericOpenGLRenderWindow>::New();
    renderer_ = vtkSmartPointer<vtkRenderer>::New();
    renderer_->SetBackground(0.06, 0.09, 0.13);
    renderer_->SetBackground2(0.14, 0.19, 0.25);
    renderer_->GradientBackgroundOn();
    renderWindow_->AddRenderer(renderer_);
    viewport_->setRenderWindow(renderWindow_);

    plasmaActor_ = visualization::actorFor(visualization::surfaceData(nullptr, 0, 1.0), visualization::colorValues(visualization::ColorScheme::Plasma));
    renderer_->AddActor(plasmaActor_);
    coilsActor_ = visualization::actorFor(visualization::schematicCoils(), visualization::colorValues(visualization::ColorScheme::Coils));
    renderer_->AddActor(coilsActor_);
    guideActor_ = visualization::actorFor(vtkSmartPointer<vtkPolyData>::New(), visualization::colorValues(visualization::ColorScheme::GuideField));
    guideActor_->GetProperty()->SetLineWidth(3.0);
    renderer_->AddActor(guideActor_);
    const auto sphere = vtkSmartPointer<vtkSphereSource>::New();
    sphere->SetRadius(0.075);
    sphere->Update();
    markerActor_ = visualization::actorFor(sphere->GetOutput(), visualization::colorValues(visualization::ColorScheme::Marker));
    renderer_->AddActor(markerActor_);

    // Particle actors container (initialized empty)
    simParticleActors_.clear();
}

void MainWindow::openResult()
{
    const QString path = QFileDialog::getOpenFileName(this, "Open VMEC output", {}, "VMEC NetCDF (*.nc);;All files (*)");
    if (!path.isEmpty())
    {
        loadResult(path);
    }
}

void MainWindow::openInput()
{
    const QString path = QFileDialog::getOpenFileName(this, "Open VMEC input", {}, "VMEC input (input.* *.json);;All files (*)");
    if (path.isEmpty())
    {
        return;
    }
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QMessageBox::warning(this, "Input error", file.errorString());
        return;
    }
    inputPath_->setText(path);
    inputEditor_->setPlainText(QString::fromUtf8(file.readAll()));
    readInputControls();
    appendLog("Loaded input: " + path);
}

void MainWindow::readInputControls()
{
    const QString source = inputEditor_->toPlainText();
    const auto flux = readInputParameter(source, "PHIEDGE");
    const auto current = readInputParameter(source, "CURTOR");
    const auto poloidal = readInputParameter(source, "MPOL");
    const auto toroidal = readInputParameter(source, "NTOR");
    const bool validFlux = flux && *flux >= fluxSpin_->minimum() && *flux <= fluxSpin_->maximum();
    const bool validCurrent = current && *current >= currentSpin_->minimum() && *current <= currentSpin_->maximum();
    const bool validPoloidal = poloidal && *poloidal >= poloidalModesSpin_->minimum() && *poloidal <= poloidalModesSpin_->maximum() && std::trunc(*poloidal) == *poloidal;
    const bool validToroidal = toroidal && *toroidal >= toroidalModesSpin_->minimum() && *toroidal <= toroidalModesSpin_->maximum() && std::trunc(*toroidal) == *toroidal;
    fluxSpin_->setEnabled(validFlux);
    currentSpin_->setEnabled(validCurrent);
    poloidalModesSpin_->setEnabled(validPoloidal);
    toroidalModesSpin_->setEnabled(validToroidal);
    if (validFlux)
    {
        fluxSpin_->setValue(*flux);
    }
    if (validCurrent)
    {
        currentSpin_->setValue(*current);
    }
    if (validPoloidal)
    {
        poloidalModesSpin_->setValue(static_cast<int>(*poloidal));
    }
    if (validToroidal)
    {
        toroidalModesSpin_->setValue(static_cast<int>(*toroidal));
    }
    inputDirty_.fill(false);
    statusBar()->showMessage("VMEC controls loaded from existing INDATA parameters; missing parameters are disabled.");
}

void MainWindow::applyInputControls()
{
    QString source = inputEditor_->toPlainText();
    int changed = 0;
    auto apply = [&source, &changed, this](const size_t index, const QWidget* control, const QString& name, const QString& value)
    {
        if (!control->isEnabled() || !inputDirty_[index])
        {
            return true;
        }
        if (!replaceInputParameter(source, name, value))
        {
            QMessageBox::warning(this, "Input changed", name + " was removed from the editor. Read values from the editor again.");
            return false;
        }
        ++changed;
        return true;
    };
    if (!apply(0, fluxSpin_, "PHIEDGE", QString::number(fluxSpin_->value(), 'g', 12)) || !apply(1, currentSpin_, "CURTOR", QString::number(currentSpin_->value(), 'g', 12)) ||
        !apply(2, poloidalModesSpin_, "MPOL", QString::number(poloidalModesSpin_->value())) || !apply(3, toroidalModesSpin_, "NTOR", QString::number(toroidalModesSpin_->value())))
    {
        return;
    }
    if (changed == 0)
    {
        statusBar()->showMessage("No VMEC parameter changes to apply.");
        return;
    }
    inputEditor_->setPlainText(source);
    inputDirty_.fill(false);
    statusBar()->showMessage(QString("Applied %1 VMEC input parameters to the editor.").arg(changed));
}

void MainWindow::loadResult(const QString& path)
{
    try
    {
        equilibrium_ = io::loadVmecEquilibrium(path.toStdString());
        if (particleTimer_.isActive())
        {
            stopParticlesButton_->setEnabled(false);
            startParticlesButton_->setEnabled(true);
            stopParticleSimulation();
        }
        vmecField_.reset();
        sourceLabel_->setText("VMEC EQUILIBRIUM\n" + QFileInfo(path).fileName());
        rebuildSurface();
        rebuildGuide();
        updateMetrics();
        renderer_->ResetCamera();
        renderWindow_->Render();
        statusBar()->showMessage("Loaded " + path);
    }
    catch (const std::exception& error)
    {
        QMessageBox::warning(this, "Result error", error.what());
    }
}

void MainWindow::runVmec()
{
    if (runner_.isRunning())
    {
        return;
    }
    if (inputPath_->text().isEmpty() || inputEditor_->toPlainText().isEmpty())
    {
        QMessageBox::information(this, "Input required", "Open a VMEC input file first.");
        return;
    }
    log_->clear();
    const jobs::VmecRunRequest request{inputPath_->text(), inputEditor_->toPlainText(), solverPath_->text(), solverThreadsSpin_->value()};
    const auto runDirectory = runner_.start(request);
    if (!runDirectory.succeeded())
    {
        QMessageBox::warning(this, "Run error", runDirectory.error);
        return;
    }
    appendLog("Run directory: " + runDirectory.runDirectory);
    appendLog(QString("Solver CPU thread limit: %1").arg(solverThreadsSpin_->value()));
    runButton_->setEnabled(false);
    stopButton_->setEnabled(true);
    statusBar()->showMessage("VMEC++ running...");
}

void MainWindow::finishVmec(const bool cancelled, const QString& resultPath, const QString& message)
{
    runButton_->setEnabled(true);
    stopButton_->setEnabled(false);
    appendLog(message);
    statusBar()->showMessage(message);
    if (cancelled || resultPath.isEmpty())
    {
        return;
    }
    loadResult(resultPath);
}

void MainWindow::rebuildSurface() const
{
    const int surface = equilibrium_ ? std::max(1, (equilibrium_->radialSurfaces - 1) * surfaceSlider_->value() / 100) : 0;
    const auto data = visualization::surfaceData(equilibrium_ ? &*equilibrium_ : nullptr, surface, surfaceSlider_->value() / 100.0);
    vtkPolyDataMapper::SafeDownCast(plasmaActor_->GetMapper())->SetInputData(data);
}

void MainWindow::rebuildGuide()
{
    guidePath_.clear();
    const int surface = equilibrium_ ? std::max(1, (equilibrium_->radialSurfaces - 1) * surfaceSlider_->value() / 100) : 0;
    const double transform = equilibrium_ && !equilibrium_->rotationalTransform.empty() ? equilibrium_->rotationalTransform[static_cast<size_t>(surface)] : 1.0;
    const auto points = vtkSmartPointer<vtkPoints>::New();
    const auto lines = vtkSmartPointer<vtkCellArray>::New();
    constexpr int Count = 640;
    lines->InsertNextCell(Count);
    for (int index = 0; index < Count; ++index)
    {
        const double phi = 4.0 * simulation::constants::PI * index / (Count - 1);
        const double theta = 0.35 + transform * phi;
        auto point = visualization::position(equilibrium_ ? &*equilibrium_ : nullptr, surface, theta, phi, surfaceSlider_->value() / 100.0);
        guidePath_.push_back(point);
        lines->InsertCellPoint(points->InsertNextPoint(point.data()));
    }
    const auto data = vtkSmartPointer<vtkPolyData>::New();
    data->SetPoints(points);
    data->SetLines(lines);
    vtkPolyDataMapper::SafeDownCast(guideActor_->GetMapper())->SetInputData(data);
    markerIndex_ = 0;
    if (!guidePath_.empty())
    {
        markerActor_->SetPosition(guidePath_.front().data());
    }
}

void MainWindow::updateMarker()
{
    if (guidePath_.empty() || !guideCheck_->isChecked() || !isVisible() || isMinimized())
    {
        return;
    }
    markerActor_->SetPosition(guidePath_[markerIndex_].data());
    markerIndex_ = (markerIndex_ + 2) % guidePath_.size();
    renderWindow_->Render();
}

void MainWindow::updateMetrics() const
{
    if (!equilibrium_)
    {
        metricsLabel_->setText("Schematic geometry. Coils and plasma shape are illustrative. "
                               "Open a VMEC result for calculated values.");
        return;
    }
    const int surface = std::max(1, (equilibrium_->radialSurfaces - 1) * surfaceSlider_->value() / 100);
    QString text =
        QString("Field periods: %1\nRadial surfaces: %2\nFourier modes: %3").arg(equilibrium_->fieldPeriods).arg(equilibrium_->radialSurfaces).arg(equilibrium_->modes.size());
    if (!equilibrium_->rotationalTransform.empty())
    {
        text += QString("\nRotational transform: %1").arg(equilibrium_->rotationalTransform[static_cast<size_t>(surface)], 0, 'g', 5);
    }
    if (equilibrium_->plasmaVolume > 0.0)
    {
        text += QString("\nPlasma volume: %1 m³").arg(equilibrium_->plasmaVolume, 0, 'g', 6);
    }
    if (equilibrium_->betaTotal > 0.0)
    {
        text += QString("\nTotal β: %1%").arg(equilibrium_->betaTotal * 100.0, 0, 'g', 4);
    }
    text += equilibrium_->hasField()
                ? QString("\nField data: %1 Nyquist modes (particle field %2)").arg(equilibrium_->fieldModes.size()).arg(vmecField_ ? "ready" : "built on first start")
                : QString("\nField data: not in this file, particles unavailable");
    text += "\n\nStationary equilibrium. The animated path is illustrative, "
            "not a traced field line or plasma velocity.";
    metricsLabel_->setText(text);
}

void MainWindow::appendLog(const QString& message) const
{
    log_->appendPlainText(message.trimmed());
}

bool MainWindow::ensureVmecField()
{
    if (vmecField_)
    {
        return true;
    }
    if (!equilibrium_ || !equilibrium_->hasField())
    {
        QMessageBox::information(this, "Field data required", "Open a VMEC result that contains bsupumnc and bsupvmnc to simulate particles in its field.");
        return false;
    }
    statusBar()->showMessage("Building magnetic field grid...");
    QString errorText;
    {
        WaitCursor busy;
        setEnabled(false);
        const auto reenable = qScopeGuard([this]() { setEnabled(true); });
        QCoreApplication::processEvents();
        try
        {
            vmecField_ = std::make_shared<const simulation::VmecFieldProvider>(*equilibrium_, simulation::VmecGridSpec{32, 96, 32});
        }
        catch (const std::exception& error)
        {
            errorText = error.what();
        }
    }
    if (!errorText.isEmpty())
    {
        QMessageBox::warning(this, "Field error", errorText);
        return false;
    }
    statusBar()->showMessage("Magnetic field grid ready.");
    updateMetrics();
    return true;
}

bool MainWindow::startParticleSimulation()
{
    if (!ensureVmecField())
    {
        return false;
    }
    simParticles_.clear();
    simParticleActors_.clear();
    simParticleAlive_.clear();

    std::mt19937_64 rng(123456);
    std::uniform_real_distribution<double> rhoDist(StartRhoMin, StartRhoMax);
    std::uniform_real_distribution<double> angleDist(0.0, 2.0 * simulation::constants::PI);
    std::normal_distribution<double> gauss(0.0, 1.0);
    const double speed = std::sqrt(2.0 * ParticleEnergyEv * simulation::constants::JOULES_PER_EV / simulation::constants::PROTON_MASS);

    const auto sphere = vtkSmartPointer<vtkSphereSource>::New();
    sphere->SetRadius(0.12);
    sphere->SetThetaResolution(20);
    sphere->SetPhiResolution(20);
    sphere->Update();

    double strongestField = 0.0;
    for (int i = 0; i < ParticleCount; ++i)
    {
        simulation::Particle p;
        p.m = simulation::constants::PROTON_MASS;
        p.q = simulation::constants::ELEMENTARY_CHARGE;
        p.x = vmecField_->position({rhoDist(rng), angleDist(rng), angleDist(rng)});

        // Isotropic direction at fixed kinetic energy...
        std::array<double, 3> direction{gauss(rng), gauss(rng), gauss(rng)};
        const double length = std::sqrt(direction[0] * direction[0] + direction[1] * direction[1] + direction[2] * direction[2]);
        for (int axis = 0; axis < 3; ++axis)
        {
            p.v[axis] = speed * direction[axis] / length;
        }

        const auto b = vmecField_->B(p.x);
        strongestField = std::max(strongestField, std::sqrt(b[0] * b[0] + b[1] * b[1] + b[2] * b[2]));

        simParticles_.push_back(p);
        simParticleAlive_.push_back(1);
        auto actor = visualization::actorFor(sphere->GetOutput(), visualization::colorValues(visualization::ColorScheme::Renderer));
        actor->GetProperty()->SetColor(1.0, 0.85, 0.1);
        actor->GetProperty()->SetAmbient(0.7);
        actor->GetProperty()->SetDiffuse(0.5);
        actor->SetPosition(p.x.data());
        renderer_->AddActor(actor);
        simParticleActors_.push_back(actor);
    }

    // Resolve the fastest gyration (strongest field) with StepsPerGyroperiod Boris steps....
    const double gyroperiod = 2.0 * simulation::constants::PI * simulation::constants::PROTON_MASS / (simulation::constants::ELEMENTARY_CHARGE * strongestField);
    particleDt_ = gyroperiod / StepsPerGyroperiod;
    particleFrame_ = 0;
    appendLog(QString("Particles: %1 protons, %2 eV, dt = %3 ns, %4 steps per frame")
                  .arg(ParticleCount)
                  .arg(ParticleEnergyEv)
                  .arg(particleDt_ * 1e9, 0, 'g', 4)
                  .arg(ParticleSubstepsPerFrame));
    particleTimer_.start();
    renderWindow_->Render();
    return true;
}

void MainWindow::stopParticleSimulation()
{
    particleTimer_.stop();
    for (auto& a : simParticleActors_)
    {
        if (a)
        {
            renderer_->RemoveActor(a);
        }
    }
    simParticleActors_.clear();
    simParticles_.clear();
    simParticleAlive_.clear();
    renderWindow_->Render();
}

void MainWindow::particleStep()
{
    if (simParticles_.empty() || !vmecField_)
    {
        return;
    }

    const bool rel = relativisticCheck_ && relativisticCheck_->isChecked();
    int confined = 0;
    for (size_t i = 0; i < simParticles_.size(); ++i)
    {
        if (!simParticleAlive_[i])
        {
            continue;
        }
        for (int sub = 0; sub < ParticleSubstepsPerFrame; ++sub)
        {
            simulation::BorisIntegrator::step(simParticles_[i], particleDt_, *vmecField_, rel);
        }
        auto& actor = simParticleActors_[i];
        if (!vmecField_->contains(simParticles_[i].x))
        {
            simParticleAlive_[i] = 0; // left the last closed flux surface
            if (actor)
            {
                actor->VisibilityOff();
            }
            continue;
        }
        ++confined;
        if (actor)
        {
            actor->SetPosition(simParticles_[i].x.data());
        }
    }
    if (++particleFrame_ % 15 == 0)
    {
        const int total = static_cast<int>(simParticles_.size());
        statusBar()->showMessage(QString("Particles: %1 confined, %2 lost, t = %3 µs")
                                     .arg(confined)
                                     .arg(total - confined)
                                     .arg(particleFrame_ * ParticleSubstepsPerFrame * particleDt_ * 1e6, 0, 'g', 4));
    }
    renderWindow_->Render();
}

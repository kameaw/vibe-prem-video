#include "app/MainWindow.h"

#include "media/MediaProbe.h"
#include "ui/PlaceholderDock.h"

#include <QFileDialog>
#include <QKeySequence>
#include <QLabel>
#include <QMenuBar>
#include <QMessageBox>
#include <QStatusBar>
#include <QVBoxLayout>

namespace vibe {

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
{
    setWindowTitle(QStringLiteral("vibe-prem-video"));
    resize(1440, 900);

    auto* center = new QWidget(this);
    auto* centerLayout = new QVBoxLayout(center);
    centerLayout->setContentsMargins(0, 0, 0, 0);
    auto* hint = new QLabel(
        QStringLiteral("Phase 0 — dock shell. Open a media file to run FFmpeg probe."),
        center);
    hint->setAlignment(Qt::AlignCenter);
    hint->setStyleSheet(QStringLiteral("color: #888; font-size: 14px;"));
    centerLayout->addWidget(hint);
    setCentralWidget(center);

    buildMenus();
    buildDocks();
    buildStatusBar();
}

void MainWindow::buildMenus()
{
    auto* fileMenu = menuBar()->addMenu(QStringLiteral("&File"));
    fileMenu->addAction(QStringLiteral("&Open Media…"), QKeySequence::Open, this,
                        &MainWindow::onOpenMedia);
    fileMenu->addSeparator();
    fileMenu->addAction(QStringLiteral("E&xit"), QKeySequence::Quit, this, &QWidget::close);

    auto* helpMenu = menuBar()->addMenu(QStringLiteral("&Help"));
    helpMenu->addAction(QStringLiteral("&About"), this, &MainWindow::onAbout);
}

void MainWindow::buildDocks()
{
    m_binDock = new PlaceholderDock(QStringLiteral("Project Bin"),
                                    QStringLiteral("Media bins and assets"), this);
    m_sourceDock = new PlaceholderDock(QStringLiteral("Source Monitor"),
                                       QStringLiteral("Preview source clips"), this);
    m_programDock = new PlaceholderDock(QStringLiteral("Program Monitor"),
                                        QStringLiteral("Sequence playback"), this);
    m_timelineDock = new PlaceholderDock(QStringLiteral("Timeline"),
                                         QStringLiteral("Multi-track editing"), this);
    m_effectsDock = new PlaceholderDock(QStringLiteral("Effect Controls"),
                                        QStringLiteral("Clip effects and keyframes"), this);

    addDockWidget(Qt::LeftDockWidgetArea, m_binDock);
    addDockWidget(Qt::RightDockWidgetArea, m_sourceDock);
    splitDockWidget(m_sourceDock, m_programDock, Qt::Horizontal);
    addDockWidget(Qt::BottomDockWidgetArea, m_timelineDock);
    addDockWidget(Qt::RightDockWidgetArea, m_effectsDock);

    // Premiere-ish starting layout: bin left, monitors top-right, effects under
    // program, timeline bottom spanning.
    splitDockWidget(m_programDock, m_effectsDock, Qt::Vertical);
    resizeDocks({m_binDock}, {280}, Qt::Horizontal);
    resizeDocks({m_timelineDock}, {260}, Qt::Vertical);
}

void MainWindow::buildStatusBar()
{
    m_statusLabel = new QLabel(QStringLiteral("Ready"), this);
    statusBar()->addWidget(m_statusLabel, 1);

#if VIBE_HAS_FFMPEG
    statusBar()->addPermanentWidget(new QLabel(QStringLiteral("FFmpeg: on"), this));
#else
    statusBar()->addPermanentWidget(new QLabel(QStringLiteral("FFmpeg: off"), this));
#endif
}

void MainWindow::onOpenMedia()
{
    const QString path = QFileDialog::getOpenFileName(
        this,
        QStringLiteral("Open Media"),
        QString(),
        QStringLiteral("Media Files (*.mp4 *.mov *.mkv *.avi *.wav *.mp3);;All Files (*.*)"));

    if (path.isEmpty()) {
        return;
    }

    const MediaProbeResult result = MediaProbe::probe(path);
    m_statusLabel->setText(result.summary);

    if (!result.ok) {
        QMessageBox::warning(this, QStringLiteral("Probe failed"), result.summary);
        return;
    }

    QMessageBox::information(this, QStringLiteral("Media probe"), result.detail);
}

void MainWindow::onAbout()
{
    QMessageBox::about(
        this,
        QStringLiteral("About vibe-prem-video"),
        QStringLiteral(
            "<b>vibe-prem-video</b> 0.1.0<br>"
            "Windows NLE shell — Phase 0<br><br>"
            "C++20 · Qt 6 · FFmpeg · Direct3D 11 (later)"));
}

} // namespace vibe

#pragma once

#include <QMainWindow>

class QLabel;

namespace vibe {

class PlaceholderDock;
class SourceMonitorWidget;

class MainWindow final : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);

private slots:
    void onOpenMedia();
    void onAbout();

private:
    void buildMenus();
    void buildDocks();
    void buildStatusBar();

    PlaceholderDock* m_binDock = nullptr;
    PlaceholderDock* m_sourceDock = nullptr;
    PlaceholderDock* m_programDock = nullptr;
    PlaceholderDock* m_timelineDock = nullptr;
    PlaceholderDock* m_effectsDock = nullptr;
    SourceMonitorWidget* m_sourceMonitor = nullptr;
    QLabel* m_statusLabel = nullptr;
};

} // namespace vibe

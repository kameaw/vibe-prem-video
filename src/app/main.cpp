#include "app/MainWindow.h"

#include <QApplication>
#include <QColor>
#include <QPalette>
#include <QStyleFactory>

namespace {

constexpr auto kAppStyleSheet = R"(
QMainWindow {
    background-color: #1e1e1e;
}

/* Grip strips between dock panels */
QMainWindow::separator {
    background: #111111;
    width: 5px;
    height: 5px;
}
QMainWindow::separator:hover {
    background: #3a7ebd;
}

QSplitter::handle {
    background: #111111;
}
QSplitter::handle:horizontal {
    width: 5px;
}
QSplitter::handle:vertical {
    height: 5px;
}
QSplitter::handle:hover {
    background: #3a7ebd;
}

QDockWidget {
    color: #dcdcdc;
    border: 1px solid #0f0f0f;
    titlebar-close-icon: none;
    titlebar-normal-icon: none;
}
QDockWidget::title {
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                stop:0 #3a3a3d, stop:1 #2b2b2e);
    padding: 7px 10px;
    border-bottom: 1px solid #0f0f0f;
    text-align: left;
    font-weight: 600;
}
QDockWidget > QWidget {
    background-color: #252526;
    border-top: 1px solid #3c3c3c;
}

QMenuBar {
    background-color: #2b2b2e;
    border-bottom: 1px solid #0f0f0f;
    padding: 2px;
}
QMenuBar::item:selected {
    background: #3a7ebd;
}
QStatusBar {
    background-color: #2b2b2e;
    border-top: 1px solid #0f0f0f;
    color: #b0b0b0;
}
)";

void applyDarkTheme(QApplication& app)
{
    app.setStyle(QStyleFactory::create(QStringLiteral("Fusion")));

    QPalette palette;
    palette.setColor(QPalette::Window, QColor(37, 37, 38));
    palette.setColor(QPalette::WindowText, QColor(220, 220, 220));
    palette.setColor(QPalette::Base, QColor(30, 30, 30));
    palette.setColor(QPalette::AlternateBase, QColor(45, 45, 48));
    palette.setColor(QPalette::ToolTipBase, QColor(37, 37, 38));
    palette.setColor(QPalette::ToolTipText, QColor(220, 220, 220));
    palette.setColor(QPalette::Text, QColor(220, 220, 220));
    palette.setColor(QPalette::Button, QColor(45, 45, 48));
    palette.setColor(QPalette::ButtonText, QColor(220, 220, 220));
    palette.setColor(QPalette::BrightText, QColor(255, 80, 80));
    palette.setColor(QPalette::Highlight, QColor(58, 126, 189));
    palette.setColor(QPalette::HighlightedText, QColor(255, 255, 255));
    palette.setColor(QPalette::Link, QColor(58, 126, 189));
    palette.setColor(QPalette::Light, QColor(60, 60, 63));
    palette.setColor(QPalette::Mid, QColor(17, 17, 17));
    palette.setColor(QPalette::Dark, QColor(15, 15, 15));
    palette.setColor(QPalette::Shadow, QColor(0, 0, 0));
    palette.setColor(QPalette::Disabled, QPalette::Text, QColor(110, 110, 110));
    palette.setColor(QPalette::Disabled, QPalette::ButtonText, QColor(110, 110, 110));
    palette.setColor(QPalette::Disabled, QPalette::WindowText, QColor(110, 110, 110));
    app.setPalette(palette);
    app.setStyleSheet(QLatin1String(kAppStyleSheet));
}

} // namespace

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    QApplication::setApplicationName(QStringLiteral("vibe-prem-video"));
    QApplication::setOrganizationName(QStringLiteral("vibe-prem"));
    QApplication::setApplicationVersion(QStringLiteral("0.1.0"));

    applyDarkTheme(app);

    vibe::MainWindow window;
    window.show();

    return app.exec();
}

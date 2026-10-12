#include "app/MainWindow.h"

#include <QApplication>
#include <QColor>
#include <QPalette>
#include <QStyleFactory>

namespace {

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
    palette.setColor(QPalette::Highlight, QColor(0, 122, 204));
    palette.setColor(QPalette::HighlightedText, QColor(255, 255, 255));
    palette.setColor(QPalette::Link, QColor(0, 122, 204));
    palette.setColor(QPalette::Disabled, QPalette::Text, QColor(110, 110, 110));
    palette.setColor(QPalette::Disabled, QPalette::ButtonText, QColor(110, 110, 110));
    palette.setColor(QPalette::Disabled, QPalette::WindowText, QColor(110, 110, 110));
    app.setPalette(palette);
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

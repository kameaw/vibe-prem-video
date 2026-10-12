#include "ui/PlaceholderDock.h"

#include <QLabel>
#include <QVBoxLayout>

namespace vibe {

PlaceholderDock::PlaceholderDock(const QString& title, const QString& subtitle, QWidget* parent)
    : QDockWidget(title, parent)
{
    setObjectName(title);
    setAllowedAreas(Qt::AllDockWidgetAreas);
    setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable
                | QDockWidget::DockWidgetClosable);

    auto* body = new QWidget(this);
    body->setObjectName(QStringLiteral("dockBody"));
    body->setStyleSheet(QStringLiteral(
        "#dockBody {"
        "  background-color: #252526;"
        "  border-left: 1px solid #3c3c3c;"
        "  border-right: 1px solid #1a1a1a;"
        "  border-bottom: 1px solid #1a1a1a;"
        "}"));

    auto* layout = new QVBoxLayout(body);
    layout->setContentsMargins(14, 14, 14, 14);
    layout->setSpacing(8);

    auto* titleLabel = new QLabel(title, body);
    titleLabel->setStyleSheet(QStringLiteral(
        "font-size: 15px; font-weight: 600; color: #e8e8e8; background: transparent;"));

    auto* subtitleLabel = new QLabel(subtitle, body);
    subtitleLabel->setWordWrap(true);
    subtitleLabel->setStyleSheet(QStringLiteral(
        "color: #9a9a9a; background: transparent;"));

    layout->addWidget(titleLabel);
    layout->addWidget(subtitleLabel);
    layout->addStretch(1);

    setWidget(body);
}

} // namespace vibe

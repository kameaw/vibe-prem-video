#include "ui/PlaceholderDock.h"

#include <QLabel>
#include <QVBoxLayout>

namespace vibe {

PlaceholderDock::PlaceholderDock(const QString& title, const QString& subtitle, QWidget* parent)
    : QDockWidget(title, parent)
{
    setObjectName(title);
    setAllowedAreas(Qt::AllDockWidgetAreas);

    auto* body = new QWidget(this);
    auto* layout = new QVBoxLayout(body);
    layout->setContentsMargins(12, 12, 12, 12);

    auto* titleLabel = new QLabel(title, body);
    titleLabel->setStyleSheet(QStringLiteral("font-size: 15px; font-weight: 600;"));

    auto* subtitleLabel = new QLabel(subtitle, body);
    subtitleLabel->setWordWrap(true);
    subtitleLabel->setStyleSheet(QStringLiteral("color: #9a9a9a;"));

    layout->addWidget(titleLabel);
    layout->addWidget(subtitleLabel);
    layout->addStretch(1);

    setWidget(body);
}

} // namespace vibe

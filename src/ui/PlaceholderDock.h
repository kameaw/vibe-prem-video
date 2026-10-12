#pragma once

#include <QDockWidget>

namespace vibe {

class PlaceholderDock final : public QDockWidget
{
    Q_OBJECT

public:
    PlaceholderDock(const QString& title, const QString& subtitle, QWidget* parent = nullptr);
};

} // namespace vibe

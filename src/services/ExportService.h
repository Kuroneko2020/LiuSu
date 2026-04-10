#pragma once

#include <QObject>

namespace pte {

class ExportService : public QObject {
    Q_OBJECT
public:
    explicit ExportService(QObject *parent = nullptr);

    Q_INVOKABLE bool exportCurrentPagePlaceholder();
    Q_INVOKABLE bool exportQueuePlaceholder();
};

} // namespace pte

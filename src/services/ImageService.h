#pragma once

#include <QObject>
#include <QStringList>

namespace pte {

class ImageService : public QObject {
    Q_OBJECT
public:
    explicit ImageService(QObject *parent = nullptr);

    Q_INVOKABLE QString importSinglePlaceholder();
    Q_INVOKABLE QStringList importBatchPlaceholder(int count);

    [[nodiscard]] QStringList supportedInputFormats() const;
};

} // namespace pte

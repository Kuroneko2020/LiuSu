#pragma once

#include <QObject>
#include <QStringList>

namespace pte {

class ImageService : public QObject {
    Q_OBJECT
public:
    explicit ImageService(QObject *parent = nullptr);

    Q_INVOKABLE QString importSingleImage();
    Q_INVOKABLE QStringList importMultipleImages();

    [[nodiscard]] QStringList supportedInputFormats() const;
    [[nodiscard]] QString fileDialogFilter() const;
    [[nodiscard]] QString heifSupportNote() const;
};

} // namespace pte

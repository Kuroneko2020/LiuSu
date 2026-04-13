#pragma once

#include <QString>
#include <QtGlobal>

namespace pte {

struct ImageResource {
    QString originalPath;
    QString originalBaseName;
    QString previewPath;
    QString exportPath;
    qint64 originalLastModifiedMs = 0;
    int previewWidth = 0;
    int previewHeight = 0;
};

} // namespace pte

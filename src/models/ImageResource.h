#pragma once

#include <QString>

namespace pte {

struct ImageResource {
    QString originalPath;
    QString originalBaseName;
    QString previewPath;
    QString exportPath;
};

} // namespace pte

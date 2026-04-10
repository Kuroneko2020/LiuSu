#include "services/ExportService.h"

namespace pte {

ExportService::ExportService(QObject *parent)
    : QObject(parent)
{
}

bool ExportService::exportCurrentPagePlaceholder()
{
    return true;
}

bool ExportService::exportQueuePlaceholder()
{
    return true;
}

} // namespace pte

#include "PagePreviewProvider.h"

#include "AppController.h"

PagePreviewProvider::PagePreviewProvider(AppController* controller)
    : QQuickImageProvider(QQuickImageProvider::Image)
    , m_controller(controller)
{
}

QImage PagePreviewProvider::requestImage(const QString& id, QSize* size, const QSize& requestedSize)
{
    // id 形如 "page/0?rev=3"；版本参数只影响 QML 缓存，不参与渲染。
    QString clean = id;
    const int queryPos = clean.indexOf(QLatin1Char('?'));
    if (queryPos >= 0)
        clean = clean.left(queryPos);

    const QStringList parts = clean.split(QLatin1Char('/'));
    const int pageIndex = parts.size() > 1 ? parts.at(1).toInt() : 0;

    const QImage image = m_controller->renderPageForProvider(pageIndex, requestedSize);
    if (size)
        *size = image.size();
    return image;
}

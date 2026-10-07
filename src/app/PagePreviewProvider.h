#pragma once

#include <QImage>
#include <QQuickImageProvider>

class AppController;

// 页面预览图像提供器：QML 以 "image://liusu/page/<页号>?rev=<版本>" 取图。
// - 版本号只用于让 QML 缓存失效；实际像素总是当前文档状态；
// - 按请求尺寸推导 PPI（48–300 夹取），让缩略图与大图各自拿到合适的清晰度；
// - 渲染走 PageRenderer 同一条管线（预览与导出同源）。
class PagePreviewProvider final : public QQuickImageProvider
{
public:
    explicit PagePreviewProvider(AppController* controller);

    QImage requestImage(const QString& id, QSize* size, const QSize& requestedSize) override;

private:
    AppController* m_controller;
};

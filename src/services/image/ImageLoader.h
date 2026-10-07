#pragma once

#include <QImage>
#include <QSize>
#include <QString>

namespace liusu::services {

// 统一图片读取入口（图片与缓存规则）：
// - EXIF 方向在此一次性应用（autoTransform），预览、缩略图、导出共用本入口；
// - UI 层与渲染层不得自行实现方向修正。
// 已知限制：Qt 的 PNG handler 不应用 eXIf 方向块（JPEG/TIFF 等正常）。
// 带方向块的 PNG 极为罕见（相机不产出），如未来真实遇到，再在本入口补
// 手动解析，禁止在调用侧各自修补。
// 渲染算法版本：进入所有缓存 key；任何影响像素输出的算法变更必须递增此值，
// 并同步评估相关缓存失效（图片与缓存规则·失效策略）。
class ImageLoader {
public:
    static constexpr int kRenderAlgorithmVersion = 1;

    struct LoadedImage {
        QImage image;
        QString error;
        bool ok = false;
    };

    // 读取已按 EXIF 方向校正的完整图像。失败时 ok==false 且 error 非空。
    static LoadedImage loadOriented(const QString& absolutePath);

    struct SourceInfo {
        bool readable = false;
        QByteArray format;      // 检测到的格式（jpeg/png/...）
        QSize orientedSize;     // 已按 EXIF 方向校正后的像素尺寸
        qint64 fileSizeBytes = -1;
        qint64 lastModifiedMs = -1;
        QString error;
    };

    // 只读元信息（不解码全部像素）。方向尺寸已按 EXIF 校正。
    static SourceInfo readInfo(const QString& absolutePath);
};

} // namespace liusu::services

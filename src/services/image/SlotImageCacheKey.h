#pragma once

#include "domain/Page.h"

#include <QString>

namespace liusu::services {

using liusu::domain::FillMode;

// 槽位图像缓存 key（图片与缓存规则·缓存 key 最低要求）。
//
// 必须覆盖的组成（缺一即可能误命中）：
// - 源身份：绝对路径 + 文件大小 + 修改时间（同路径替换必然失效）；
// - 预览目标尺寸 previewMaxEdge；
// - EXIF 方向：已在解码层统一应用（ImageLoader），此处以"渲染算法版本 +
//   源身份"表达方向结果的等价版本标识；
// - 旋转 / 镜像 / 填充模式 / 裁切偏移；
// - 渲染算法版本 ImageLoader::kRenderAlgorithmVersion。
//
// 页面级参数（模板 id/参数、边框 id/参数、背景）属于页面合成缓存（子方案 04），
// 在彼处的 key 中并入；本 key 覆盖到"单张已变换的槽位图"为止。
//
// toString() 产生稳定的 `|` 分隔字符串，直接作为缓存键使用；
// 字段顺序即契约，追加新字段必须追加在末尾并递增渲染算法版本评估。
struct SlotImageCacheKey {
    QString imagePath;
    qint64 fileSizeBytes = -1;
    qint64 lastModifiedMs = -1;
    int previewMaxEdge = 0;
    int rotationDegrees = 0;
    bool mirrored = false;
    FillMode fillMode = FillMode::Fill;
    qreal cropOffsetX = 0.0;
    qreal cropOffsetY = 0.0;
    int renderVersion = 0;

    QString toString() const;
};

} // namespace liusu::services

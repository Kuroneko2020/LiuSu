#include "ImageLoader.h"

#include <QFileInfo>
#include <QImageReader>

namespace liusu::services {

ImageLoader::LoadedImage ImageLoader::loadOriented(const QString& absolutePath)
{
    LoadedImage result;
    QImageReader reader(absolutePath);
    // EXIF 方向修正只允许发生在这里（图片与缓存规则）。
    reader.setAutoTransform(true);
    result.image = reader.read();
    if (result.image.isNull()) {
        result.error = reader.errorString();
        return result;
    }
    result.ok = true;
    return result;
}

ImageLoader::SourceInfo ImageLoader::readInfo(const QString& absolutePath)
{
    SourceInfo info;
    const QFileInfo fileInfo(absolutePath);
    if (!fileInfo.exists() || !fileInfo.isFile()) {
        info.error = QStringLiteral("文件不存在: %1").arg(absolutePath);
        return info;
    }
    info.fileSizeBytes = fileInfo.size();
    info.lastModifiedMs = fileInfo.lastModified().toMSecsSinceEpoch();

    QImageReader reader(absolutePath);
    reader.setAutoTransform(true);
    info.format = reader.format();
    if (!reader.canRead()) {
        info.error = reader.errorString();
        return info;
    }
    info.readable = true;
    // size() 不含方向变换；按 handler 声明的变换推出校正后尺寸
    // （旋转/转置类变换会交换宽高，镜像类不交换）。
    const QSize rawSize = reader.size();
    const auto transformations = reader.transformation();
    using T = QImageIOHandler::Transformation;
    const auto swapMask = T::TransformationRotate90 | T::TransformationMirrorAndRotate90
        | T::TransformationFlipAndRotate90 | T::TransformationRotate270;
    QSize orientedSize = rawSize;
    if (transformations & swapMask)
        orientedSize.transpose();
    info.orientedSize = orientedSize;
    return info;
}

} // namespace liusu::services

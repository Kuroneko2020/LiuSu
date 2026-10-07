# 服务层：图片读取、EXIF、变换、缓存。
# 允许依赖 Qt6::Gui（QImage）与领域层；不得依赖 QtQuick / UI。
qt_add_library(liusu_services STATIC
    image/ImageLoader.cpp
    image/SlotImageCacheKey.cpp
    image/ThumbnailCache.cpp
    render/PageRenderer.cpp
)

target_link_libraries(liusu_services PUBLIC liusu_domain Qt6::Gui)
target_include_directories(liusu_services PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)

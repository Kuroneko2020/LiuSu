cmake_minimum_required(VERSION 3.21)

project(LiuSu VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# QuickControls2：QML 控件与界面风格（QQuickStyle）的依赖；05 子方案定风格时在此管理。
find_package(Qt6 6.5 REQUIRED COMPONENTS Quick QuickControls2 QuickShapes QuickEffects)
qt_standard_project_setup(REQUIRES 6.5)

# 应用壳：QML 界面入口。领域模型（src/domain）、服务层（src/services）
# 随子方案 02/03 落地时各自建立 CMake 目标，业务代码不得回填进 app 壳。
qt_add_executable(LiuSu
    src/app/main.cpp
    src/app/AppTheme.h
    src/app/AppController.h
    src/app/AppController.cpp
    src/app/PagePreviewProvider.h
    src/app/PagePreviewProvider.cpp
)

qt_add_qml_module(LiuSu
    URI LiuSu
    VERSION 1.0
    QML_FILES
        qml/Main.qml
        qml/AcrylicPanel.qml
        qml/AcrylicToolButton.qml
        qml/LayoutBoard.qml
        qml/HomePage.qml
        qml/EditorPage.qml
)

# 预览演示图：EXIF 方向测试图集嵌入资源，供渲染管线可视化验证。
set(LIUSU_PREVIEW_IMAGES
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/fixtures/exif1.jpg
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/fixtures/exif3.jpg
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/fixtures/exif6.jpg
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/fixtures/exif8.jpg
)
foreach(img ${LIUSU_PREVIEW_IMAGES})
    get_filename_component(img_name ${img} NAME)
    set_source_files_properties(${img} PROPERTIES QT_RESOURCE_ALIAS ${img_name})
endforeach()
qt_add_resources(LiuSu "preview_images"
    PREFIX "/images"
    FILES ${LIUSU_PREVIEW_IMAGES}
)

target_link_libraries(LiuSu PRIVATE Qt6::Quick Qt6::QuickControls2 Qt6::QuickShapes Qt6::QuickEffects)
target_link_libraries(LiuSu PRIVATE liusu_services)

# 领域层独立于应用壳构建，供应用与测试共同链接。
add_subdirectory(src/domain)
add_subdirectory(src/services)

enable_testing()
add_subdirectory(tests)

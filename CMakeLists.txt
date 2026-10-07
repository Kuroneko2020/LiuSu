cmake_minimum_required(VERSION 3.21)

project(LiuSu VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# QuickControls2：QML 控件与界面风格（QQuickStyle）的依赖；05 子方案定风格时在此管理。
find_package(Qt6 6.5 REQUIRED COMPONENTS Quick QuickControls2)
qt_standard_project_setup(REQUIRES 6.5)

# 应用壳：QML 界面入口。领域模型（src/domain）、服务层（src/services）
# 随子方案 02/03 落地时各自建立 CMake 目标，业务代码不得回填进 app 壳。
qt_add_executable(LiuSu
    src/app/main.cpp
)

qt_add_qml_module(LiuSu
    URI LiuSu
    VERSION 1.0
    QML_FILES
        qml/Main.qml
)

target_link_libraries(LiuSu PRIVATE Qt6::Quick Qt6::QuickControls2)

enable_testing()
add_subdirectory(tests)

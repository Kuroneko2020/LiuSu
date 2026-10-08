# 领域层：页面/打印机档案、布局模型、项目文件。
# 只允许依赖 Qt6::Core——几何真相不得沾染 UI 与渲染细节。
qt_add_library(liusu_domain STATIC
    Units.cpp
    Layout.cpp
    Page.cpp
    ProjectDocument.cpp
    PageOperations.cpp
)

target_link_libraries(liusu_domain PUBLIC Qt6::Core)

# 对外以 "domain/xxx.h" 形式引用（include 根是 src/）。
target_include_directories(liusu_domain PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)

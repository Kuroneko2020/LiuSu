cmake_minimum_required(VERSION 3.21)
project(PhotoTemplateEditor VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Gui Qml Quick QuickControls2 Widgets)

qt_standard_project_setup(REQUIRES 6.5)
qt_policy(SET QTP0004 NEW)

qt_add_executable(photo_template_editor
    src/main.cpp
    src/app/AppController.cpp
    src/models/ProjectState.cpp
    src/services/ImageService.cpp
    src/services/ExportService.cpp
    src/services/TemplateLayout.cpp
    src/services/AutoLayoutPolicy.cpp
)

target_include_directories(photo_template_editor PRIVATE src)

target_link_libraries(photo_template_editor PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Qml
    Qt6::Quick
    Qt6::QuickControls2
    Qt6::Widgets
)

qt_add_qml_module(photo_template_editor
    URI PhotoTemplateEditor
    VERSION 1.0
    QML_FILES
        qml/Main.qml
        qml/pages/HomePage.qml
        qml/pages/EditorPage.qml
        qml/pages/ExportPage.qml
        qml/pages/SettingsPage.qml
        qml/components/TemplateCard.qml
        qml/components/IconToolButton.qml
        qml/components/TemplateCanvas.qml
        qml/components/SlotItem.qml
        qml/components/FilmStrip.qml
    RESOURCES
        res/icons/rotate-right.svg
        res/icons/mirror.svg
        res/icons/fit-inside.svg
        res/icons/fill-crop.svg
        res/icons/adjust.svg
        res/icons/delete.svg
        res/icons/slot-plus.svg
        res/icons/import.svg
        res/icons/page-add.svg
        res/icons/export-one.svg
        res/icons/export-all.svg
        res/icons/page-delete.svg
)

install(TARGETS photo_template_editor
    BUNDLE DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

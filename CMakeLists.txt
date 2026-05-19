cmake_minimum_required(VERSION 3.21)
project(Liusu VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

include(GNUInstallDirs)

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Gui Qml Quick QuickControls2 Widgets)

qt_standard_project_setup(REQUIRES 6.5)
set(LIUSU_QT_VERSION_FOR_POLICIES "")
if(DEFINED QT_VERSION)
    set(LIUSU_QT_VERSION_FOR_POLICIES "${QT_VERSION}")
elseif(DEFINED Qt6_VERSION)
    set(LIUSU_QT_VERSION_FOR_POLICIES "${Qt6_VERSION}")
endif()
if(LIUSU_QT_VERSION_FOR_POLICIES VERSION_GREATER_EQUAL "6.8.0")
    qt_policy(SET QTP0004 NEW)
endif()

qt_add_executable(liusu
    src/main.cpp
    src/app/AppController.cpp
    src/models/ProjectState.cpp
    src/services/ImageService.cpp
    src/services/ExportService.cpp
    src/services/TemplateLayout.cpp
    src/services/AutoLayoutPolicy.cpp
)

target_include_directories(liusu PRIVATE src)

target_link_libraries(liusu PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Qml
    Qt6::Quick
    Qt6::QuickControls2
    Qt6::Widgets
)

qt_add_qml_module(liusu
    URI Liusu
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
        qml/components/PageTopBar.qml
        qml/components/ColorPickerDialog.qml
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

install(TARGETS liusu
    BUNDLE DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

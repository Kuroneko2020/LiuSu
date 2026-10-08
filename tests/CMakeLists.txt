find_package(Qt6 6.5 REQUIRED COMPONENTS Test)

qt_add_executable(tst_smoke smoke/tst_smoke.cpp)
target_link_libraries(tst_smoke PRIVATE Qt6::Test)

add_test(NAME smoke COMMAND tst_smoke)

add_subdirectory(domain)
add_subdirectory(image)
add_subdirectory(render)

qt_add_executable(tst_catalog domain/tst_catalog.cpp)
target_link_libraries(tst_catalog PRIVATE Qt6::Test liusu_domain)
add_test(NAME catalog COMMAND tst_catalog)

qt_add_executable(tst_app app/tst_app.cpp ../src/app/AppController.cpp ../src/app/AppController.h)
target_link_libraries(tst_app PRIVATE Qt6::Gui Qt6::Test liusu_services)
add_test(NAME app COMMAND tst_app)

qt_add_executable(tst_ui app/tst_ui.cpp ../src/app/AppController.cpp ../src/app/AppController.h
    ../src/app/AppTheme.h ../src/app/PagePreviewProvider.cpp)
target_link_libraries(tst_ui PRIVATE Qt6::Test Qt6::Quick Qt6::QuickControls2 liusu_services)
target_compile_definitions(tst_ui PRIVATE LIUSU_SOURCE_DIR="${PROJECT_SOURCE_DIR}")
set_source_files_properties("${PROJECT_SOURCE_DIR}/assets/photos/coast.png" PROPERTIES QT_RESOURCE_ALIAS "coast.png")
qt_add_resources(tst_ui "test_studio_photos" PREFIX "/photos" FILES "${PROJECT_SOURCE_DIR}/assets/photos/coast.png")
file(GLOB LIUSU_TEST_QML CONFIGURE_DEPENDS "${PROJECT_SOURCE_DIR}/qml/*.qml")
qt_add_resources(tst_ui "test_studio_qml" PREFIX "/qt/qml/LiuSu" BASE "${PROJECT_SOURCE_DIR}" FILES ${LIUSU_TEST_QML})
add_test(NAME ui COMMAND tst_ui)

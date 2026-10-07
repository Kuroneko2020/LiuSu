find_package(Qt6 6.5 REQUIRED COMPONENTS Gui Test)

qt_add_executable(tst_image tst_image.cpp)
target_link_libraries(tst_image PRIVATE Qt6::Gui Qt6::Test liusu_services)

add_test(NAME image COMMAND tst_image)

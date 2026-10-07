find_package(Qt6 6.5 REQUIRED COMPONENTS Gui Test)

qt_add_executable(tst_render tst_render.cpp)
target_link_libraries(tst_render PRIVATE Qt6::Gui Qt6::Test liusu_services)

add_test(NAME render COMMAND tst_render)

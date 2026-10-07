find_package(Qt6 6.5 REQUIRED COMPONENTS Test)

qt_add_executable(tst_smoke smoke/tst_smoke.cpp)
target_link_libraries(tst_smoke PRIVATE Qt6::Test)

add_test(NAME smoke COMMAND tst_smoke)

add_subdirectory(domain)
add_subdirectory(image)

find_package(Qt6 6.5 REQUIRED COMPONENTS Test)

qt_add_executable(tst_domain tst_domain.cpp)
target_link_libraries(tst_domain PRIVATE Qt6::Test liusu_domain)

add_test(NAME domain COMMAND tst_domain)

# Comparison test executables
# These tests compare C++ implementation with Go implementation

find_package(GTest REQUIRED CONFIG)

# Go vs C++ comparison test
add_executable(test_go_vs_cpp test_go_vs_cpp.cpp)

target_link_libraries(test_go_vs_cpp
    PRIVATE
    GTest::gtest
    GTest::gtest_main
    optimum_p2p_client
)

add_test(NAME test_go_vs_cpp COMMAND test_go_vs_cpp)

set_tests_properties(test_go_vs_cpp PROPERTIES
    TIMEOUT 30
    LABELS "comparison"
)

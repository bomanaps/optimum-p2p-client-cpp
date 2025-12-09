# Unit test executables

# Find Google Test using modern config
find_package(GTest REQUIRED CONFIG)

# Test utilities
add_executable(test_utils test_utils.cpp)

target_link_libraries(test_utils
    PRIVATE
    GTest::gtest
    GTest::gtest_main
    optimum_p2p_client
)

# Add test to CTest
add_test(NAME test_utils COMMAND test_utils)

# Set test properties
set_tests_properties(test_utils PROPERTIES
    TIMEOUT 30
)

# Test hex utilities
add_executable(test_utils_hex test_utils_hex.cpp)

target_link_libraries(test_utils_hex
    PRIVATE
    GTest::gtest
    GTest::gtest_main
    optimum_p2p_client
)

add_test(NAME test_utils_hex COMMAND test_utils_hex)

set_tests_properties(test_utils_hex PROPERTIES
    TIMEOUT 30
)

# Integration test executables
# These tests require Docker/P2P nodes to be running

# Find Google Test using modern config
find_package(GTest REQUIRED CONFIG)

# Single client integration tests
add_executable(test_single_client test_single_client.cpp)

target_link_libraries(test_single_client
    PRIVATE
    GTest::gtest
    GTest::gtest_main
    optimum_p2p_client
)

# Add test to CTest (disabled by default - requires running nodes)
add_test(NAME test_single_client COMMAND test_single_client)

# Mark as disabled - requires external setup
set_tests_properties(test_single_client PROPERTIES
    DISABLED TRUE
    TIMEOUT 60
    LABELS "integration;requires_docker"
)

# Multi-client integration tests
add_executable(test_multi_client test_multi_client.cpp)

target_link_libraries(test_multi_client
    PRIVATE
    GTest::gtest
    GTest::gtest_main
    optimum_p2p_client
)

add_test(NAME test_multi_client COMMAND test_multi_client)

set_tests_properties(test_multi_client PROPERTIES
    DISABLED TRUE
    TIMEOUT 120
    LABELS "integration;requires_docker"
)

# Proxy client integration tests
add_executable(test_proxy_client test_proxy_client.cpp)

target_link_libraries(test_proxy_client
    PRIVATE
    GTest::gtest
    GTest::gtest_main
    optimum_p2p_client
)

add_test(NAME test_proxy_client COMMAND test_proxy_client)

set_tests_properties(test_proxy_client PROPERTIES
    DISABLED TRUE
    TIMEOUT 120
    LABELS "integration;requires_docker"
)

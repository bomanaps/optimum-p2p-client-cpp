# End-to-end test executables
# These tests require full Docker setup with P2P nodes and Proxy

find_package(GTest REQUIRED CONFIG)

# Full workflow test
add_executable(test_full_workflow test_full_workflow.cpp)

target_link_libraries(test_full_workflow
    PRIVATE
    GTest::gtest
    GTest::gtest_main
    optimum_p2p_client
)

add_test(NAME test_full_workflow COMMAND test_full_workflow)

set_tests_properties(test_full_workflow PROPERTIES
    DISABLED TRUE
    TIMEOUT 300
    LABELS "e2e;requires_docker;requires_proxy"
)


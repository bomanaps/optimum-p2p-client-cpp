# Test configuration

# Include directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../include)
include_directories(${CMAKE_BINARY_DIR}/../proto)

# Unit tests
add_subdirectory(unit)

# Integration tests (if enabled)
if(BUILD_INTEGRATION_TESTS)
    add_subdirectory(integration)
endif()

# End-to-end tests (if enabled)
if(BUILD_E2E_TESTS)
    add_subdirectory(e2e)
endif()

# Comparison tests (if enabled)
if(BUILD_COMPARISON_TESTS)
    add_subdirectory(comparison)
endif()

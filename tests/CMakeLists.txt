# Add Google Test
include(FetchContent)
FetchContent_Declare(
	googletest
    URL https://github.com/google/googletest/releases/download/v1.15.2/googletest-1.15.2.tar.gz
	DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# Add test executable
add_executable(mnxdom_tests
    test_main.cpp
)

if (CMAKE_CXX_COMPILER_ID MATCHES "AppleClang|Clang|GNU")
    # Extra warnings for Clang/AppleClang
    target_compile_options(mnxdom_tests PRIVATE -Wall -Wextra -Werror -Wpedantic)
elseif (CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    # Extra warnings for MSVC
    target_compile_options(mnxdom_tests PRIVATE /Zc:__cplusplus /W4 /WX)
endif()

target_include_directories(mnxdom_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(mnxdom_tests PRIVATE
    gtest_main
    mnxdom
)

# Discover tests
include(GoogleTest)
gtest_discover_tests(mnxdom_tests
    DISCOVERY_TIMEOUT 60 # Set timeout to 60 seconds
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/data
    PROPERTIES
        ENVIRONMENT "GTEST_COLOR=1"
    EXTRA_ARGS --gtest_color=yes
)

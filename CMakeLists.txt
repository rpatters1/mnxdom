cmake_minimum_required(VERSION 3.14)
project(mnxdom LANGUAGES CXX)

include(FetchContent)

# Set default build type to Release
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type (Release/Debug)" FORCE)
endif()

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if (MSVC)
    add_compile_options(/bigobj)
endif()

# Add library
add_library(mnxdom INTERFACE)
target_include_directories(mnxdom INTERFACE src)

# Disable building tests for nlohmann_json
set(JSON_BuildTests OFF CACHE BOOL "Do not build tests for nlohmann_json")
# Define the version of nlohmann_json to use
set(NLOHMANN_JSON_VERSION v3.11.3)
# Fetch nlohmann_json
FetchContent_Declare(
    nlohmann_json
    URL https://github.com/nlohmann/json/releases/download/${NLOHMANN_JSON_VERSION}/json.tar.xz
    URL_HASH SHA256=d6c65aca6b1ed68e7a182f4757257b107ae403032760ed6ef121c9d55e81757d  # Verify the hash matches the tarball
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
FetchContent_MakeAvailable(nlohmann_json)
target_link_libraries(mnxdom INTERFACE nlohmann_json::nlohmann_json)

# Allow users to disable building tests
option(mnxdom_BUILD_TESTING "Enable testing for your library" ON)
if(mnxdom_BUILD_TESTING)
    message(STATUS "Testing enabled for mnxdom_BUILD_TESTING.")
    enable_testing()
    add_subdirectory(tests)
else()
    message(STATUS "Testing not enabled for mnxdom_BUILD_TESTING.")
endif()

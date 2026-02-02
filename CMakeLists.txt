cmake_minimum_required(VERSION 3.14)
project(mnxdom LANGUAGES CXX)

include(FetchContent)

# Set default build type to Release
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type (Release/Debug)" FORCE)
endif()

# Set the C++ standard
if(NOT CMAKE_CXX_STANDARD)
    set(CMAKE_CXX_STANDARD 23)
endif()

# Enforce at least C++17, allow higher if set by the parent
if(CMAKE_CXX_STANDARD LESS 17)
    message(FATAL_ERROR "Mnxdom requires at least C++17. Current: C++${CMAKE_CXX_STANDARD}")
endif()

message(STATUS "Mnxdom C++ standard: ${CMAKE_CXX_STANDARD}")

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if (MSVC)
    add_compile_options(/bigobj)
endif()

#define other directories
set(GENERATED_DIR "${CMAKE_BINARY_DIR}/generated")
set(MNX_SCHEMA_DIR "${CMAKE_CURRENT_LIST_DIR}/schema")

include("${CMAKE_CURRENT_LIST_DIR}/cmake/MnxW3C.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/cmake/GenerateMnxSchemaXxd.cmake")

# Add library
add_library(mnxdom STATIC
    src/EnumerationMaps.cpp
    src/Implementations.cpp
    src/validation/SchemaValidate.cpp
    src/validation/SemanticValidate.cpp
)

target_include_directories(mnxdom PUBLIC src)

# Disable building tests for nlohmann_json
set(JSON_BuildTests OFF CACHE BOOL "Do not build tests for nlohmann_json")
# Define the version of nlohmann_json to use
set(NLOHMANN_JSON_VERSION v3.12.0)
# Fetch nlohmann_json
FetchContent_Declare(
    nlohmann_json
    URL https://github.com/nlohmann/json/releases/download/${NLOHMANN_JSON_VERSION}/json.tar.xz
    URL_HASH SHA256=42f6e95cad6ec532fd372391373363b62a14af6d771056dbfc86160e6dfff7aa  # Verify the hash matches the tarball
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
FetchContent_MakeAvailable(nlohmann_json)

# Disable building tests for json_schema_validator
set(JSON_VALIDATOR_BUILD_TESTS OFF CACHE BOOL "Do not build tests for json_schema_validator")
# Fetch json_schema_validator from your fork
set(nlohmann_json_VERSION ${NLOHMANN_JSON_VERSION})
FetchContent_Declare(
    json_schema_validator
    GIT_REPOSITORY https://github.com/pboettch/json-schema-validator
    GIT_TAG        40af3ec39670e768fc3f01f935140af311d71024
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
FetchContent_MakeAvailable(json_schema_validator)

target_link_libraries(mnxdom PUBLIC nlohmann_json::nlohmann_json)
target_link_libraries(mnxdom PUBLIC nlohmann_json_schema_validator)
# nlohmann/json behavior config
target_compile_definitions(mnxdom PUBLIC JSON_DISABLE_ENUM_SERIALIZATION=1)
# Export fetched mnx_w3c locations to dependents
mnx_w3c_apply_interface_definitions(mnxdom)
target_compile_options(mnxdom PUBLIC
    $<$<CXX_COMPILER_ID:Clang,AppleClang>:-Wno-gnu-zero-variadic-macro-arguments>
)
if (MSVC)
    # Needed for __VA_OPT__ support in macro-heavy headers on MSVC.
    target_compile_options(mnxdom PUBLIC /Zc:preprocessor)
endif()

add_dependencies(mnxdom GenerateMnxSchemaXxd)
target_include_directories(mnxdom PRIVATE ${GENERATED_DIR})

# Allow users to disable building tests
option(mnxdom_BUILD_TESTING "Enable testing mnxdom" ON)
if(mnxdom_BUILD_TESTING)
    message(STATUS "Testing enabled for mnxdom_BUILD_TESTING.")
    enable_testing()
    add_subdirectory(tests)
else()
    message(STATUS "Testing not enabled for mnxdom_BUILD_TESTING.")
endif()

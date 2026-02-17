include(FetchContent)

# Handle mnx_w3c: use local source directory or fetch
if(MNX_W3C_SOURCE AND EXISTS "${MNX_W3C_SOURCE}")
    set(mnx_w3c_SOURCE_DIR "${MNX_W3C_SOURCE}")
    message(STATUS "Using local mnx_w3c from: ${mnx_w3c_SOURCE_DIR}")
else()
    # Fetch MNX spec
    FetchContent_Declare(
        mnx_w3c
        GIT_REPOSITORY https://github.com/w3c/mnx.git
        GIT_TAG d513cf7a28a84c803ed36e448b416cb49c3a22f9
        SOURCE_SUBDIR _cmake_disabled_please_ignore # this is a hack to prevent FetchContent_Declare from running the fetched content cmake.
        # see https://stackoverflow.com/questions/79261625/cmake-fetchcontent-with-header-only-project/79261858#79261858
    )
    FetchContent_MakeAvailable(mnx_w3c)
    message(STATUS "Using fetched mnx_w3c at: ${mnx_w3c_SOURCE_DIR}")
endif()

set(MNX_W3C_SCHEMA_PATH "${mnx_w3c_SOURCE_DIR}/docs/mnx-schema.json")
set(MNX_W3C_EXAMPLES_PATH "${mnx_w3c_SOURCE_DIR}/docs/static/examples/json")

function(mnx_w3c_apply_interface_definitions target_name)
    if(NOT TARGET "${target_name}")
        message(FATAL_ERROR "mnx_w3c_apply_interface_definitions: target '${target_name}' does not exist")
    endif()

    target_compile_definitions("${target_name}" PUBLIC
        MNX_W3C_SCHEMA_PATH="${MNX_W3C_SCHEMA_PATH}"
        MNX_W3C_EXAMPLES_PATH="${MNX_W3C_EXAMPLES_PATH}"
    )
endfunction()

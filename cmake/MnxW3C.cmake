include(FetchContent)

# Fetch MNX spec
FetchContent_Declare(
    mnx_w3c
    GIT_REPOSITORY git@github.com:w3c/mnx.git
    GIT_TAG 75551875790062bc9d61e3ca468a050136021f58
    SOURCE_SUBDIR _cmake_disabled_please_ignore # this is a hack to prevent FetchContent_Declare from running the fetched content cmake.
    # see https://stackoverflow.com/questions/79261625/cmake-fetchcontent-with-header-only-project/79261858#79261858
)
FetchContent_MakeAvailable(mnx_w3c)
message(STATUS "mnx_w3c fetched at: ${mnx_w3c_SOURCE_DIR}")

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

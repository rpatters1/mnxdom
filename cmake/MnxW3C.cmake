if(NOT MNX_W3C_SOURCE)
    get_filename_component(MNX_W3C_SOURCE "${CMAKE_CURRENT_LIST_DIR}/../third_party/w3c-mnx" ABSOLUTE)
endif()

set(mnx_w3c_SOURCE_DIR "${MNX_W3C_SOURCE}")

if(EXISTS "${mnx_w3c_SOURCE_DIR}/mnx-schema.json")
    set(MNX_W3C_SCHEMA_PATH "${mnx_w3c_SOURCE_DIR}/mnx-schema.json")
elseif(EXISTS "${mnx_w3c_SOURCE_DIR}/docs/mnx-schema.json")
    set(MNX_W3C_SCHEMA_PATH "${mnx_w3c_SOURCE_DIR}/docs/mnx-schema.json")
else()
    message(FATAL_ERROR "w3c-mnx schema not found under MNX_W3C_SOURCE: ${mnx_w3c_SOURCE_DIR}")
endif()

if(EXISTS "${mnx_w3c_SOURCE_DIR}/examples")
    set(MNX_W3C_EXAMPLES_PATH "${mnx_w3c_SOURCE_DIR}/examples")
elseif(EXISTS "${mnx_w3c_SOURCE_DIR}/docs/static/examples/json")
    set(MNX_W3C_EXAMPLES_PATH "${mnx_w3c_SOURCE_DIR}/docs/static/examples/json")
else()
    message(FATAL_ERROR "w3c-mnx examples not found under MNX_W3C_SOURCE: ${mnx_w3c_SOURCE_DIR}")
endif()

message(STATUS "Using w3c-mnx schema: ${MNX_W3C_SCHEMA_PATH}")
message(STATUS "Using w3c-mnx examples: ${MNX_W3C_EXAMPLES_PATH}")

function(mnx_w3c_apply_interface_definitions target_name)
    if(NOT TARGET "${target_name}")
        message(FATAL_ERROR "mnx_w3c_apply_interface_definitions: target '${target_name}' does not exist")
    endif()

    target_compile_definitions("${target_name}" PUBLIC
        MNX_W3C_SCHEMA_PATH="${MNX_W3C_SCHEMA_PATH}"
        MNX_W3C_EXAMPLES_PATH="${MNX_W3C_EXAMPLES_PATH}"
    )
endfunction()

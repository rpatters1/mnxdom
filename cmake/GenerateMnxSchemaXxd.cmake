# GenerateMnxSchemaXxd.cmake

# Define the paths
set(MNX_SCHEMA_JSON "${MNX_W3C_SCHEMA_PATH}")
set(GENERATED_MNX_XXD "${GENERATED_DIR}/mnx_schema.xxd")
get_filename_component(MNX_SCHEMA_DIR "${MNX_SCHEMA_JSON}" DIRECTORY)
find_program(XXD_EXECUTABLE xxd REQUIRED)

# Step 2: Convert mnx-schema.json to mnx_schema.xxd
add_custom_command(
    OUTPUT "${GENERATED_MNX_XXD}"
    COMMAND ${CMAKE_COMMAND} -E echo "Generating mnx_schema.xxd..."
    COMMAND ${CMAKE_COMMAND} -E make_directory "${GENERATED_DIR}"
    COMMAND ${CMAKE_COMMAND} -E chdir "${MNX_SCHEMA_DIR}"
            "${XXD_EXECUTABLE}" -i "mnx-schema.json" "${GENERATED_MNX_XXD}"
    DEPENDS "${MNX_SCHEMA_JSON}"
    COMMENT "Converting mnx-schema.json to mnx_schema.xxd"
    VERBATIM
)

# Step 3: Add the generated file as a dependency for your target
add_custom_target(
    GenerateMnxSchemaXxd ALL
    DEPENDS ${GENERATED_MNX_XXD}
)

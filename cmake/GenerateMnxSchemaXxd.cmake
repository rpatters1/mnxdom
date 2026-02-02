# GenerateMnxSchemaXxd.cmake

# Define the paths
set(MNX_SCHEMA_JSON "${MNX_W3C_SCHEMA_PATH}")
set(GENERATED_MNX_XXD "${GENERATED_DIR}/mnx_schema.xxd")
find_program(XXD_EXECUTABLE xxd REQUIRED)

# Step 2: Convert mnx-schema.json to mnx_schema.xxd
add_custom_command(
    OUTPUT "${GENERATED_MNX_XXD}"
    COMMAND ${CMAKE_COMMAND} -E echo "Generating mnx_schema.xxd..."
    COMMAND ${CMAKE_COMMAND} -E make_directory "${GENERATED_DIR}"
    COMMAND "${XXD_EXECUTABLE}" -i -n "mnx_schema_json" "${MNX_SCHEMA_JSON}" "${GENERATED_MNX_XXD}"
    DEPENDS "${MNX_SCHEMA_JSON}"
    COMMENT "Converting mnx-schema.json to mnx_schema.xxd"
    VERBATIM
)

# Step 3: Add the generated file as a dependency for your target
add_custom_target(
    GenerateMnxSchemaXxd ALL
    DEPENDS ${GENERATED_MNX_XXD}
)

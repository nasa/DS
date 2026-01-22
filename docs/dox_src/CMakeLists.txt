########################################################
#
# CMake Recipe to build doxygen documentation
#
########################################################

# This CMake script currently defines a top-level target
# to build the stand alone documentation.  This may be invoked either
# from the main CMakeLists.txt as a subdirectory (useful in the
# case of a self-contained/standalone build) or by a separate script
# (useful if integrating into a larger project with a separate doc build)

project(DS_DOCS NONE)

# generate the configuration files
configure_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/ds-common.doxyfile.in
        ${CMAKE_BINARY_DIR}/docs/ds-common.doxyfile
        @ONLY
)

configure_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/ds-usersguide.doxyfile.in
        ${CMAKE_CURRENT_BINARY_DIR}/ds-usersguide.doxyfile
        @ONLY
)

add_custom_command(OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/html/index.html" "${CMAKE_CURRENT_BINARY_DIR}/ALWAYSBUILD"
    COMMAND doxygen ${CMAKE_CURRENT_BINARY_DIR}/ds-usersguide.doxyfile
    WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
)

add_custom_target(ds-usersguide
    COMMAND echo "DS UsersGuide: file://${CMAKE_CURRENT_BINARY_DIR}/html/index.html"
    DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/ALWAYSBUILD"
)

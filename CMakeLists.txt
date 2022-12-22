project(CFS_DS C)

set(APP_SRC_FILES
  fsw/src/ds_table.c
  fsw/src/ds_cmds.c
  fsw/src/ds_app.c
  fsw/src/ds_file.c
)

# Create the app module
add_cfe_app(ds ${APP_SRC_FILES})

# This permits direct access to public headers in the fsw/inc directory
target_include_directories(ds PUBLIC fsw/inc)

set(APP_TABLE_FILES
  fsw/tables/ds_filter_tbl.c
  fsw/tables/ds_file_tbl.c
)

add_cfe_tables(ds ${APP_TABLE_FILES})

# If UT is enabled, then add the tests from the subdirectory
# Note that this is an app, and therefore does not provide
# stub functions, as other entities would not typically make
# direct function calls into this application.
if(ENABLE_UNIT_TESTS)
  add_subdirectory(unit-test)
endif()

# App specific mission scope configuration

# Add stand alone documentation
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/docs/dox_src ${MISSION_BINARY_DIR}/docs/ds-usersguide)

# The list of header files that control the DS configuration
set(DS_MISSION_CONFIG_FILE_LIST
  ds_extern_typedefs.h
  ds_fcncode_values.h
  ds_interface_cfg_values.h
  ds_mission_cfg.h
  ds_msgdefs.h
  ds_msgstruct.h
  ds_msg.h
  ds_tbldefs.h
  ds_tblstruct.h
  ds_tbl.h
  ds_topicid_values.h
)

generate_configfile_set(${DS_MISSION_CONFIG_FILE_LIST})

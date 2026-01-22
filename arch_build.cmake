###########################################################
#
# DS App platform build setup
#
# This file is evaluated as part of the "prepare" stage
# and can be used to set up prerequisites for the build,
# such as generating header files
#
###########################################################

# The list of header files that control the app configuration
set(DS_PLATFORM_CONFIG_FILE_LIST
  ds_internal_cfg_values.h
  ds_msgid_values.h
  ds_msgids.h
  ds_platform_cfg.h
)

generate_configfile_set(${DS_PLATFORM_CONFIG_FILE_LIST})

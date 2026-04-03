sdk_generate_library(log)
sdk_add_compile_options(-DLOG_LEVEL=${CONFIG_LOG_LEVEL})
sdk_add_include_directories(.)
sdk_library_add_sources(log.c)
# SPDX-License-Identifier: Apache-2.0

zephyr_library_sources_ifdef(CONFIG_ENTROPY_RT583 entropy_rt583.c)

target_include_directories(app PRIVATE
    ${RAFAEL_SDK}/components/platform/hosal/rt583_hosal/Inc
)

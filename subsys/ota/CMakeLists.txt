if(CONFIG_OTA)

# When included via include() the working dir is the caller's dir,
# so use an explicit absolute path for all sources.
# RT583_PROJECT_DIR is set by the including CMakeLists.txt (repo root)
set(OTA_DIR ${RT583_PROJECT_DIR}/subsys/ota)

target_sources(app PRIVATE
    ${OTA_DIR}/ota_coap.c
    ${OTA_DIR}/ota_flash.c
)

target_include_directories(app PUBLIC
    ${OTA_DIR}
)

# LZ4 single-file implementation
target_sources(app PRIVATE
    ${RT583_PROJECT_DIR}/sdk/components/utils/lz4/lz4.c
)
target_include_directories(app PRIVATE
    ${RT583_PROJECT_DIR}/sdk/components/utils/lz4
)

# mbedTLS SHA-256 — use OpenThread's bundled copy
set(MBEDTLS_INC
    ${RT583_PROJECT_DIR}/sdk/components/network/thread/openthread/third_party/mbedtls/repo/include
)
target_include_directories(app PRIVATE ${MBEDTLS_INC})

# mbedTLS sha256.c (compiled as part of OpenThread lib, but we need the header
# and the symbol; if the linker complains about missing sha256, add the source:)
# target_sources(app PRIVATE
#     ${RT583_PROJECT_DIR}/sdk/components/network/thread/openthread/third_party/mbedtls/repo/library/sha256.c
# )

endif()

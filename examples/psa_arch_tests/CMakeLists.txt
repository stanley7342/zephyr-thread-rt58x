cmake_minimum_required(VERSION 3.20.0)

get_filename_component(RT583_PROJECT_DIR "${CMAKE_CURRENT_LIST_DIR}/../.." ABSOLUTE)

set(BOARD_ROOT   ${RT583_PROJECT_DIR})
set(SOC_ROOT     ${RT583_PROJECT_DIR})
set(DTS_ROOT     ${RT583_PROJECT_DIR})
set(KCONFIG_ROOT ${RT583_PROJECT_DIR}/Kconfig)

list(APPEND ZEPHYR_EXTRA_MODULES ${RT583_PROJECT_DIR})

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(rt58x_psa_arch_tests VERSION 1.0.0)

# ------------------------------------------------------------------ #
# Locate psa-arch-tests source tree.                                  #
# Default: D:/matter/psa-arch-tests (matches user's local clone).     #
# Override: set PSA_ARCH_TESTS_DIR env var or -D on cmake cmdline.    #
# ------------------------------------------------------------------ #
if(DEFINED ENV{PSA_ARCH_TESTS_DIR})
    set(PSA_ARCH_TESTS_DIR $ENV{PSA_ARCH_TESTS_DIR})
elseif(NOT DEFINED PSA_ARCH_TESTS_DIR)
    set(PSA_ARCH_TESTS_DIR "D:/matter/psa-arch-tests")
endif()

if(NOT EXISTS "${PSA_ARCH_TESTS_DIR}/api-tests")
    message(FATAL_ERROR
        "psa-arch-tests not found at ${PSA_ARCH_TESTS_DIR}/api-tests.\n"
        "Clone it: git clone https://github.com/ARM-software/psa-arch-tests "
        "${PSA_ARCH_TESTS_DIR}\n"
        "Or set PSA_ARCH_TESTS_DIR env var to its location.")
endif()

set(API_TESTS  "${PSA_ARCH_TESTS_DIR}/api-tests")
set(VAL_NSPE   "${API_TESTS}/val/nspe")
set(VAL_COMMON "${API_TESTS}/val/common")
set(CRYPTO_DIR "${API_TESTS}/dev_apis/crypto")
set(COMMON_VAL "${PSA_ARCH_TESTS_DIR}/val_common")

if(NOT EXISTS "${COMMON_VAL}/inc/val_common_log.h")
    message(FATAL_ERROR
        "psa-arch-tests/val_common submodule not initialised.\n"
        "Run: cd ${PSA_ARCH_TESTS_DIR} && git submodule update --init val_common")
endif()
set(SUITE_DB   "${CRYPTO_DIR}/1.1.0_testsuite.db")
set(GEN_PY     "${API_TESTS}/tools/scripts/gen_tests_list.py")

# ------------------------------------------------------------------ #
# Step 1: Generate testlist + test_entry_list.inc + decls .inc        #
# Run gen_tests_list.py at configure-time (results checked in to      #
# build dir, regenerated on every reconfigure).                       #
# ------------------------------------------------------------------ #
set(GEN_DIR "${CMAKE_CURRENT_BINARY_DIR}/psa_gen")
file(MAKE_DIRECTORY ${GEN_DIR})

set(TESTLIST_FILE        "${GEN_DIR}/testlist.txt")
set(TEST_ENTRY_LIST_INC  "${GEN_DIR}/test_entry_list.inc")
set(TEST_ENTRY_DECL_INC  "${GEN_DIR}/test_entry_fn_declare_list.inc")
# Unused outputs for non-IPC suites; pass dummy paths.
set(GEN_DUMMY_1          "${GEN_DIR}/_dummy_client_decl.inc")
set(GEN_DUMMY_2          "${GEN_DIR}/_dummy_client_list.inc")
set(GEN_DUMMY_3          "${GEN_DIR}/_dummy_server_decl.inc")
set(GEN_DUMMY_4          "${GEN_DIR}/_dummy_server_list.inc")

execute_process(
    COMMAND python "${GEN_PY}"
        crypto                 # arg1: SUITE
        "${SUITE_DB}"          # arg2: testsuite.db
        0                      # arg3: panic_tests_included
        ALL                    # arg4: tests_coverage
        "${TESTLIST_FILE}"     # arg5
        "${TEST_ENTRY_LIST_INC}"
        "${TEST_ENTRY_DECL_INC}"
        "${GEN_DUMMY_1}"
        "${GEN_DUMMY_2}"
        "${GEN_DUMMY_3}"
        "${GEN_DUMMY_4}"
        None None              # arg12,13: test range
    RESULT_VARIABLE gen_result
    OUTPUT_VARIABLE gen_stdout
    ERROR_VARIABLE  gen_stderr
)
if(NOT gen_result EQUAL 0)
    message(FATAL_ERROR "gen_tests_list.py failed: ${gen_stderr}")
endif()
message(STATUS "[psa-arch-tests] Generated test_entry_list.inc (${gen_stdout})")

# ------------------------------------------------------------------ #
# Step 2: Read generated testlist.txt → list of test_cNNN dirs.       #
# ------------------------------------------------------------------ #
file(STRINGS "${TESTLIST_FILE}" RAW_TEST_LINES)
set(PSA_TEST_LIST "")
foreach(line ${RAW_TEST_LINES})
    string(REGEX MATCH "^test_c[0-9]+" tc "${line}")
    if(tc)
        list(APPEND PSA_TEST_LIST ${tc})
    endif()
endforeach()
list(LENGTH PSA_TEST_LIST PSA_TEST_COUNT)
message(STATUS "[psa-arch-tests] ${PSA_TEST_COUNT} crypto tests selected")

# ------------------------------------------------------------------ #
# Step 3: Collect all source files.                                   #
# ------------------------------------------------------------------ #
set(PSA_SRCS
    # val framework (NSPE side only — we're not running IPC partitions)
    ${VAL_NSPE}/val_crypto.c
    ${VAL_NSPE}/val_dispatcher.c
    ${VAL_NSPE}/val_entry.c
    ${VAL_NSPE}/val_framework.c
    ${VAL_NSPE}/val_interfaces.c
    ${VAL_NSPE}/val_peripherals.c
    ${VAL_NSPE}/val_platform.c
    ${VAL_NSPE}/val_storage.c
    # crypto common helpers
    ${CRYPTO_DIR}/common/test_crypto_common.c
    # platform-common PAL bits (vendor's stock NSPE pieces)
    ${API_TESTS}/platform/targets/common/nspe/pal_weak.c
    ${API_TESTS}/platform/targets/common/nspe/crypto/pal_crypto_intf.c
    # common val (separate submodule — log/status/framework/peripherals helpers)
    ${COMMON_VAL}/src/val_common_log.c
    ${COMMON_VAL}/src/val_common_status.c
    ${COMMON_VAL}/src/val_common_framework.c
    ${COMMON_VAL}/src/val_common_peripherals.c
    # our target-specific PAL impl
    ${CMAKE_CURRENT_SOURCE_DIR}/pal/pal_driver_intf.c
    # zephyr app entry
    ${CMAKE_CURRENT_SOURCE_DIR}/src/main.c
)
foreach(test ${PSA_TEST_LIST})
    # File convention: dir = "test_c001", sources = "test_c001.c" + "test_entry_c001.c"
    string(REGEX REPLACE "^test_" "" tnum "${test}")  # test_c001 -> c001
    list(APPEND PSA_SRCS
        ${CRYPTO_DIR}/${test}/${test}.c
        ${CRYPTO_DIR}/${test}/test_entry_${tnum}.c
    )
endforeach()

target_sources(app PRIVATE ${PSA_SRCS})

# ------------------------------------------------------------------ #
# Step 4: Include directories.                                        #
# ------------------------------------------------------------------ #
target_include_directories(app PRIVATE
    ${VAL_COMMON}
    ${VAL_NSPE}
    ${CRYPTO_DIR}/common
    ${API_TESTS}/platform/targets/common/nspe          # pal_common.h, pal_interfaces_ns.h
    ${API_TESTS}/platform/targets/common/nspe/crypto   # crypto-side common
    ${COMMON_VAL}/inc                                  # val_common_log.h etc
    ${CMAKE_CURRENT_SOURCE_DIR}/pal
    ${GEN_DIR}                    # for test_entry_list.inc
)
foreach(test ${PSA_TEST_LIST})
    target_include_directories(app PRIVATE ${CRYPTO_DIR}/${test})
endforeach()

# ------------------------------------------------------------------ #
# Step 5: Compile flags.                                              #
# ------------------------------------------------------------------ #
target_compile_definitions(app PRIVATE
    CRYPTO                          # selects #ifdef CRYPTO blocks in val + pal
    SHA256_TRUNCATED_DIGEST_NOT_SUPPORTED  # mbedTLS SHA-256 truncated NOT impl
    # val_common_log.c defaults to 64-bit-host static_assert checks; on
    # 32-bit Cortex-M sizeof(long)=4 not 8, so skip them.
    STATIC_ASSERT_CHECKS=1
    # Compile-time knobs the upstream build passes via -D; we hardcode here.
    VERBOSITY=3
    VAL_NSPE_BUILD
    NONSECURE_TEST_BUILD
    PSA_CMAKE_BUILD                 # tell pal_config.h NOT to define those macros itself
    PLATFORM_PSA_ISOLATION_LEVEL=3  # tell tests we claim isolation level 3 (we don't really have it)
    SP_HEAP_MEM_SUPP                # vendor flag — heap available to secure partitions
)

# psa-arch-tests has its own quirks; relax -Werror for the framework code so
# warnings in vendor headers (unused params, sign-compare) don't break the
# Zephyr build.
target_compile_options(app PRIVATE
    -Wno-unused-parameter
    -Wno-unused-but-set-variable
    -Wno-sign-compare
    -Wno-implicit-fallthrough
    -Wno-strict-prototypes
)

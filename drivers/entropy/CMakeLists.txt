# SPDX-License-Identifier: Apache-2.0

# entropy_rt58{3,4}.c are wired into the drivers__entropy library via
# cmake/zephyr_module/CMakeLists.txt (so the right hosal_trng.c TU and
# include paths are picked per SoC). This file intentionally adds nothing —
# left here so the directory remains a valid Zephyr driver subdir.

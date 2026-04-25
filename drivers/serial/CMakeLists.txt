# Out-of-tree UART drivers for Rafael RT58x SoCs.
#
# The real sources + include paths are wired into Zephyr's `drivers__serial`
# library target from cmake/zephyr_module/CMakeLists.txt, which runs at the
# correct point in Zephyr's build graph (after drivers__serial is created).
#
# This file is intentionally a no-op — kept only so that a glance at
# drivers/serial/ shows that the folder is driver-aware. If Zephyr ever
# starts auto-adding this directory in the future, flip the logic back to
# use zephyr_library_sources() and remove the duplicates from
# cmake/zephyr_module/CMakeLists.txt.

#
# CPUID-related options
#
# This was added to support compiler versions which might not support all the
# flags we need
#

#find out GCC version
execute_process(
    COMMAND bash -c "echo __GNUC__ | ${CC} -E -x c - | tail -n 1"
    OUTPUT_VARIABLE GCC_MAJOR
)

execute_process(
    COMMAND bash -c "echo __GNUC_MINOR__ | ${CC} -E -x c - | tail -n 1"
    OUTPUT_VARIABLE GCC_MINOR
)

set(GCC_VERSION ${GCC_MAJOR}${GCC_MINOR})


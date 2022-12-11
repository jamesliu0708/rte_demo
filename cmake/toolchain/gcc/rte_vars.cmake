#
# toolchain:
#
#   - define CC, LD, AR, AS, ... (overridden by cmdline value)
#   - define TOOLCHAIN_CFLAGS variable (overridden by cmdline value)
#   - define TOOLCHAIN_LDFLAGS variable (overridden by cmdline value)
#   - define TOOLCHAIN_ASFLAGS variable (overridden by cmdline value)
#

set(CC gcc)
set(CPP cpp)
set(AS nasm)
set(AR ar)
set(LD ld)
set(OBJCOPY objcopy)
set(OBJDUMP objdump)
set(STRIP strip)
set(READELF readelf)
set(GCOV gcov)

set(TOOLCHAIN_ASFLAGS )
set(TOOLCHAIN_CFLAGS )
set(TOOLCHAIN_LDFLAGS )

if (NOT "${CONFIG_RTE_LIBRTE_GCOV}" STREQUAL "")
    set(TOOLCHAIN_ASFLAGS ${TOOLCHAIN_ASFLAGS}" --coverage")
    set(TOOLCHAIN_LDFLAGS ${TOOLCHAIN_LDFLAGS}" --coverage")
endif ()

set(WERROR_FLAGS "-W -Wall -Wstrict-prototypes -Wmissing-prototypes
                -Wmissing-declarations -Wold-style-definition -Wpointer-arith
                -Wcast-align -Wnested-externs -Wcast-qual
                -Wformat-nonliteral -Wformat-security
                -Wundef -Wwrite-strings -Wdeprecated")

if (NOT "${RTE_DEVEL_BUILD}" STREQUAL "")
    set(WERROR_FLAGS ${WERROR_FLAGS}" -Werror")
endif ()

# There are many issues reported for strict alignment architectures
# which are not necessarily fatal. Report as warnings.
if (NOT "$(CONFIG_RTE_ARCH_STRICT_ALIGN)" STREQUAL "")
    set(WERROR_FLAGS ${WERROR_FLAGS}" -Wno-error=cast-align")
endif ()

include (${RTE_SDK}/cmake/toolchain/gcc/rte_toolchain_compat.cmake)

execute_process(
    COMMAND bash -c "test $(GCC_VERSION) -gt 70 && echo 1"
    OUTPUT_VARIABLE TSTFLAG
)
# workaround GCC bug with warning "missing initializer" for "= {0}"
if ("${TSTFLAG}" STREQUAL "1")
    set(WERROR_FLAGS ${WERROR_FLAGS}" -Wno-missing-field-initializers")
endif ()
# workaround GCC bug with warning "may be used uninitialized"
if ("${TSTFLAG}" STREQUAL "1")
    set(WERROR_FLAGS ${WERROR_FLAGS}" -Wno-uninitialized")
endif ()
# Tell GCC only to error for switch fallthroughs without a suitable comment
if ("${TSTFLAG}" STREQUAL "1")
    set(WERROR_FLAGS ${WERROR_FLAGS}" -Wimplicit-fallthrough=2")
endif ()
# Ignore errors for snprintf truncation
if ("${TSTFLAG}" STREQUAL "1")
    set(WERROR_FLAGS ${WERROR_FLAGS}" -Wno-format-truncation")
endif ()

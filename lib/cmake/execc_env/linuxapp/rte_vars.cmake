#
# exec-env:
#
#   - define EXECENV_CFLAGS variable (overridden by cmdline)
#   - define EXECENV_LDFLAGS variable (overridden by cmdline)
#   - define EXECENV_ASFLAGS variable (overridden by cmdline)
#   - may override any previously defined variable
#
# examples for RTE_EXEC_ENV: linuxapp
#

if (${CONFIG_RTE_BUILD_SHARED_LIB})
    set(EXECENV_CFLAGS "-pthread -fPIC")
else ()
    set(EXECENV_CFLAGS pthread)
endif ()

if (${CONFIG_RTE_BUILD_SHARED_LIB})
    set(EXECENV_LDLIBS "${EXECENV_LDLIBS} lgcc_s")
endif ()
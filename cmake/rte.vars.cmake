if (NOT DEFINE RTE_SDK)
    set(RTE_SDK ${CMAKE_CURRENT_LIST_DIR})
endif ()

if (NOT DEFINE RTE_OUTPUT)
    set(RTE_SDK ${CMAKE_CURRENT_LIST_DIR})
endif()

if (NOT DEFINE RTE_CONFIG_TEMPLATE)
    execute_process(
        COMMAND bash -c "uname -m | awk '{ \
            if ($$0 == "aarch64") { \
                    print "arm64-armv8a"} \
            else if ($$0 == "armv7l") { \
                    print "arm-armv7a"} \
            else if ($$0 == "ppc64") { \
                    print "ppc_64-power8"} \
            else if ($$0 == "amd64") { \
                    print "x86_64-native"} \
            else { \
                    printf "%s-native", $$0} }'"
        OUTPUT_VARIABLE ARCH_TMP
    )
    execute_process(
        COMMAND bash -c "uname | awk '{ \
            if ($$0 == "Linux") { \
                    print "linuxapp"} \
            else { \
                    print "bsdapp"} }'"
        OUTPUT_VARIABLE OSAPP_TMP
    )
    execute_process(
        COMMAND bash -c "${CMAKE_C_COMPILER} --version | grep -o 'cc\|gcc\|icc\|clang' | awk \
		'{ \
		if ($$1 == "cc") { \
			print "gcc" } \
		else { \
			print $$1 } \
		}'"
        OUTPUT_VARIABLE COMPILER_TMP
    )
    set(RTE_CONFIG_TEMPLATE ${RTE_SDK}/config/deconfig_${ARCH_TMP}-${OSAPP_TMP}-${COMPILER_TMP})
    if (NOT EXISTS ${RTE_CONFIG_TEMPLATE})
        message(FATAL_ERROR "${RTE_CONFIG_TEMPLATE} does not exist")
    endif ()
endif ()

include (${CMAKE_CURRENT_LIST_DIR}/rte.sdkconfig.cmake)
include (${CMAKE_CURRENT_LIST_DIR}/toolchain/rte_vars.cmake)
include ()
    
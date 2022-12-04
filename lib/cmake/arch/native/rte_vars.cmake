set(MACHINE_CFLAGS native)
# On FreeBSD systems, sometimes the correct CPU type is not picked up.
# To get everything to compile, we need SSE4.2 support, so check if that is
# reported by compiler. If not, check if the CPU actually supports it, and if
# so, set the compilation target to be a corei7, minimum target with SSE4.2.
execute_process(
    COMMAND bash -c "${CC} -march=native -dM -E - </dev/null| grep SSE4_2"
    OUTPUT_VARIABLE SSE42_SUPPORT
)
if(${SSE42_SUPPORT} STREQUAL "")
    set(MACHINE_CFLAGS corei7)
endif()
execute_process(
    COMMAND bash -c "${CC} -march=${MACHINE_CFLAGS} -dM -E - </dev/null"
    OUTPUT_VARIABLE AUTO_CPUFLAGS
)
set (CPUFLAGS )
string(FIND ${AUTO_CPUFLAGS} "__SSE__" output)
if (NOT ${output} STREQUAL "-1")
    list (APPEND CPUFLAGS SSE)
endif()

string (FIND ${AUTO_CPUFLAGS} "__SSE2__" output)
if (NOT ${output} STREQUAL "-1")
    list (APPEND CPUFLAGS SSE2)
endif()

string(FIND ${AUTO_CPUFLAGS} "__SSE3__" output)
if (NOT ${output} STREQUAL "-1")
    list(APPEND CPUFLAGS SSE3)
endif()

string(FIND ${AUTO_CPUFLAGS} "__SSSE3__" output)
if (NOT ${output} STREQUAL "-1")
    list (APPEND CPUFLAGS SSSE3)
endif()

string (FIND ${AUTO_CPUFLAGS} "__SSE4_1__" output)
if (NOT ${output} STREQUAL "-1")
    list (APPEND CPUFLAGS SSSE4_1)
endif ()

string (FIND ${AUTO_CPUFLAGS} "__SSE4_2__" output)
if (NOT ${output} STREQUAL "-1")
    list (APPEND CPUFLAGS SSE4_2)
endif ()

string (FIND ${AUTO_CPUFLAGS} "__AES__" output)
if (NOT ${output} STREQUAL "-1")
    list (APPEND CPUFLAGS AES)
endif ()

string (FIND ${AUTO_CPUFLAGS} "__PCLMUL__" output)
if (NOT ${output} STREQUAL "-1")
    list (APPEND CPUFLAGS PCLMULQDQ)
endif ()

string (FIND ${AUTO_CPUFLAGS} "__AVX__" output)
if (NOT ${output} STREQUAL "-1")
    list (APPEND CPUFLAGS AVX)
endif ()

string (FIND ${AUTO_CPUFLAGS} "__RDRND__" output)
if (NOT ${output} STREQUAL "-1")
    list (APPEND CPUFLAGS RDRAND)
endif ()

string (FIND ${AUTO_CPUFLAGS} "__FSGSBASE__" output)
if (NOT ${output} STREQUAL "-1")
    list (APPEND CPUFLAGS FSGSBASE)
endif ()

string (FIND ${AUTO_CPUFLAGS} "__F16C__" output)
if (NOT ${output} STREQUAL "-1")
    list (APPEND CPUFLAGS F16C)
endif ()

string(FIND ${AUTO_CPUFLAGS} "__AVX2__" output)
if (${CONFIG_RTE_ENABLE_AVX})
    if (NOT ${output} STREQUAL "-1")
        list(APPEND CPUFLAGS AVX2)
    endif()
endif()

string(FIND ${AUTO_CPUFLAGS} "__AVX512F__" output)
if (${CONFIG_RTE_ENABLE_AVX512})
    if (NOT ${output} STREQUAL "-1")
        list(APPEND CPUFLAGS AVX512F)
    endif ()
endif()

set(MACHINE_CFLAGS )
foreach (CPUFLAG ${CPUFLAGS})
    if ("${MACHINE_CFLAGS}" STREQUAL "")
        set(MACHINE_CFLAGS -DRTE_MACHINE_CPUFLAG_${CPUFLAG})
    else ()
        set(MACHINE_CFLAGS "${MACHINE_CFLAGS} -DRTE_MACHINE_CPUFLAG_${CPUFLAG}")
    endif ()
endforeach ()

set (CPUFLAGS_LIST )
foreach (CPUFLAG ${CPUFLAGS})
    if ("${CPUFLAGS_LIST}" STREQUAL "")
        set(CPUFLAGS_LIST RTE_CPUFLAG_${CPUFLAG})
    else ()
        set(CPUFLAGS_LIST ${CPUFLAGS_LIST},RTE_CPUFLAG_${CPUFLAG})
    endif ()
endforeach ()

set(CPUFLAGS_LIST -DRTE_COMPILE_TIME_CPUFLAGS=${CPUFLAGS_LIST})
message(status ${MACHINE_CFLAGS})
message(status ${CPUFLAGS_LIST})
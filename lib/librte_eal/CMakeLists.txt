add_compile_options(-O3)

execute_process(
    COMMAND bash -c "cp ${CMAKE_CURRENT_LIST_DIR}/common/*.h ${RTE_OUTPUT}/include/"
)
aux_source_directory(${CMAKE_CURRENT_LIST_DIR}/common src)
aux_source_directory(${CMAKE_CURRENT_LIST_DIR}/linuxapp/eal eal)
list(APPEND src ${eal})
if (RTE_ARCH STREQUAL x86_64)
    execute_process(
        COMMAND bash -c "cp ${CMAKE_CURRENT_LIST_DIR}/common/arch/x86/*.h ${RTE_OUTPUT}/include"
        COMMAND bash -c "cp ${CMAKE_CURRENT_LIST_DIR}/common/include/arch/*.h ${RTE_OUTPUT}/include"
    )
    aux_source_directory(${CMAKE_CURRENT_LIST_DIR}/common/arch/x86/ x86arch)
    list(APPEND src ${x86arch})
endif ()
execute_process(
    COMMAND bash -c "cp ${CMAKE_CURRENT_LIST_DIR}/common/include/*.h ${RTE_OUTPUT}/include/"
    COMMAND bash -c "cp -r ${CMAKE_CURRENT_LIST_DIR}/common/generic ${RTE_OUTPUT}/generic"
)
if (${RTE_BUILD_SHARED_LIB} STREQUAL "y")
    add_library(rte_eal SHARED ${src})
else ()
    add_library(rte_eal STATIC ${src})
endif ()
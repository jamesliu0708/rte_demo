add_compile_options(-O3)

execute_process(
    COMMAND bash -c "cp ${CMAKE_CURRENT_LIST_DIR}/*.h ${RTE_OUTPUT}/include/"
)

aux_source_directory(${CMAKE_CURRENT_LIST_DIR} rte_mempool)

if (${RTE_BUILD_SHARED_LIB} STREQUAL "y")
    add_library(rte_mempool SHARED ${rte_mempool})
else ()
    add_library(rte_mempool STATIC ${srrte_mempoolc})
endif ()
target_include_directories(rte_mempool PRIVATE ${RTE_OUTPUT}/include/)

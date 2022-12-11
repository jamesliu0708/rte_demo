add_compile_options(-O3)

execute_process(
    COMMAND bash -c "cp ${CMAKE_CURRENT_LIST_DIR}/*.h ${RTE_OUTPUT}/include/"
)

aux_source_directory(${CMAKE_CURRENT_LIST_DIR} rte_ring)

if (${RTE_BUILD_SHARED_LIB} STREQUAL "y")
    add_library(rte_ring SHARED ${rte_ring})
else ()
    add_library(rte_ring STATIC ${rte_ring})
endif ()
target_include_directories(rte_ring PRIVATE ${RTE_OUTPUT}/include/)

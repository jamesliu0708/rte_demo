cmake_minimum_required(VERSION 3.10)
project(unitest)
add_compile_options(-mssse3)
# add_compile_options(-O3 -march=native)
add_compile_options(-O0 -ggdb)

# add_subdirectory(googletest)

add_link_options(-rdynamic)

include_directories(${CMAKE_CURRENT_LIST_DIR}/../lib/common)
include_directories(${CMAKE_CURRENT_LIST_DIR}/../lib/common/arch/x86)
include_directories(${CMAKE_CURRENT_LIST_DIR}/../lib/common/include/arch/x86)
include_directories(${CMAKE_CURRENT_LIST_DIR}/../lib/common/include)
include_directories(${CMAKE_CURRENT_LIST_DIR}/../lib/driver)
include_directories(${CMAKE_CURRENT_LIST_DIR}/../lib/)
include_directories(${CMAKE_CURRENT_LIST_DIR}/../lib/librte_mempool)
include_directories(${CMAKE_CURRENT_LIST_DIR}/../lib/linuxapp)
include_directories(${CMAKE_CURRENT_LIST_DIR}/../lib/librte_ring)

add_executable(tst_memcpy tst_memcpy.cc)
target_link_libraries(tst_memcpy gtest pthread)

add_executable(tst_memcpy_perf tst_memcpy_perf.cc)

add_executable(tst_atomic tst_atomic.cc)
target_link_libraries(tst_atomic gtest pthread)

add_executable(tst_log tst_log.cc ${CMAKE_CURRENT_LIST_DIR}/../lib/common/eal_common_log.c)

add_executable(tst_debug tst_debug.cc 
                        ${CMAKE_CURRENT_LIST_DIR}/../lib/linuxapp/eal/eal_debug.c 
                        ${CMAKE_CURRENT_LIST_DIR}/../lib/common/eal_common_log.c 
                        )
target_link_libraries(tst_debug gtest pthread)

add_executable(tst_malloc tst_malloc.cc)
target_link_directories(tst_malloc PRIVATE ${CMAKE_CURRENT_LIST_DIR}/../output)
target_link_directories(tst_malloc PRIVATE ${CMAKE_CURRENT_LIST_DIR}/../lib/build)
target_link_libraries(tst_malloc rte_demo gtest pthread)

add_executable(tst_memory tst_memory.cc)
target_link_directories(tst_memory PRIVATE ${CMAKE_CURRENT_LIST_DIR}/../lib/build)
target_link_libraries(tst_memory rte_demo pthread)

add_executable(tst_memzone tst_memzone.cc)
target_link_directories(tst_memzone PRIVATE ${CMAKE_CURRENT_LIST_DIR}/../lib/build)
target_link_libraries(tst_memzone rte_demo gtest pthread)

add_executable(tst_mempool tst_mempool.cc)
target_link_directories(tst_mempool PRIVATE ${CMAKE_CURRENT_LIST_DIR}/../lib/build)
target_link_libraries(tst_mempool rte_demo gtest pthread)

add_executable(tst_ring tst_ring.cc)
target_link_directories(tst_ring PRIVATE ${CMAKE_CURRENT_LIST_DIR}/../lib/build)
target_link_libraries(tst_ring rte_demo gtest pthread)
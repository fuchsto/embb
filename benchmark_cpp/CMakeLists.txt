project (project_embb_benchmark_cpp)

file(GLOB_RECURSE EMBB_BENCHMARK_CPP_SOURCES "src/*.cc" "src/*.h")
file(GLOB_RECURSE EMBB_BENCHMARK_CPP_HEADERS "include/*.h")

# Execute the GroupSources macro
include(${CMAKE_SOURCE_DIR}/CMakeCommon/GroupSourcesMSVC.cmake)
GroupSourcesMSVC(include)
GroupSourcesMSVC(src)

set (EMBB_BENCHMARK_CPP_INCLUDE_DIRS "include" "src")
include_directories(${EMBB_BENCHMARK_CPP_INCLUDE_DIRS}
                    ${CMAKE_CURRENT_SOURCE_DIR}/../base_c/include
                    ${CMAKE_CURRENT_BINARY_DIR}/../base_c/include
                    ${CMAKE_CURRENT_SOURCE_DIR}/../base_cpp/include
                    ${CMAKE_CURRENT_BINARY_DIR}/../base_cpp/include
                    ${CMAKE_CURRENT_SOURCE_DIR}/../base_cpp/perf/include
                    ${CMAKE_CURRENT_BINARY_DIR}/../base_cpp/perf/include
                    ${CMAKE_CURRENT_SOURCE_DIR}/../containers_cpp/include
                    ${CMAKE_CURRENT_BINARY_DIR}/../containers_cpp/include
                    ${CMAKE_CURRENT_SOURCE_DIR}/../mtapi_c/include
                    ${CMAKE_CURRENT_BINARY_DIR}/../mtapi_c/include
                    ${CMAKE_CURRENT_SOURCE_DIR}/../mtapi_cpp/include
                    ${CMAKE_CURRENT_BINARY_DIR}/../mtapi_cpp/include
                    ${EMBB_BASE_CPP_PERF_PAPI_INC}
                    )

add_executable (embb_benchmark_cpp ${EMBB_BENCHMARK_CPP_SOURCES} ${EMBB_BENCHMARK_CPP_HEADERS})
target_link_libraries(embb_benchmark_cpp
                      embb_containers_cpp
                      embb_base_cpp_perf 
                      embb_base_cpp 
                      embb_mtapi_cpp 
                      embb_mtapi_c 
                      embb_base_c 
                      ${EMBB_BASE_CPP_PERF_PAPI_LIB}
                      ${compiler_libs}
                     )

CopyBin(BIN embb_benchmark_cpp DEST ${local_install_dir})

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/embb
        DESTINATION include FILES_MATCHING PATTERN "*.h")
install(TARGETS embb_benchmark_cpp DESTINATION lib)

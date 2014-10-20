project (project_embb_benchmark_cpp)

if (USE_PERFORMANCE_API STREQUAL OFF)
  message("-- Using integrated performance counters (default)")
else()
  message("-- Using performance counters from PAPI")
endif()
message("   (set with command line option -DUSE_PERFORMANCE_API=ON/OFF)")

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
                    ${CMAKE_CURRENT_BINARY_DIR}/../mtapi_cpp/include)

add_executable (embb_benchmark_cpp ${EMBB_BENCHMARK_CPP_SOURCES} ${EMBB_BENCHMARK_CPP_HEADERS})
target_link_libraries(embb_benchmark_cpp embb_base_cpp embb_base_cpp_perf embb_containers_cpp embb_mtapi_cpp embb_mtapi_c)

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/embb
        DESTINATION include FILES_MATCHING PATTERN "*.h")
install(TARGETS embb_benchmark_cpp DESTINATION lib)

project (project_embb_base_cpp)

## CODE FILE DETECTION
#
# Fetch all header and source files for C, C++, and test build separately
file(GLOB_RECURSE EMBB_BASE_CPP_SOURCES "src/*.cc" "src/*.h")
file(GLOB_RECURSE EMBB_BASE_CPP_HEADERS "include/embb/*.h")
if (BUILD_TESTS STREQUAL ON)
  file(GLOB_RECURSE EMBB_BASE_TEST_SOURCES "test/*.cc" "test/*.h")
endif()
if (BUILD_BENCHMARKS STREQUAL ON)
  file(GLOB_RECURSE EMBB_BASE_CPP_PERF_SOURCES "perf/*.cc" "perf/*.h")
  file(GLOB_RECURSE EMBB_BASE_CPP_PERF_HEADERS "perf/include/embb/*.h")
endif()
if (USE_PERFORMANCE_API STREQUAL ON)
  message("   papi headers included from ${EMBB_BASE_CPP_PERF_PAPI_INC}")
  message("   papi library linked from   ${EMBB_BASE_CPP_PERF_PAPI_LIB}")
endif()

# Create header file from input file
configure_file("include/embb/base/internal/cmake_config.h.in" 
               "include/embb/base/internal/cmake_config.h")

# Execute the GroupSources macro
include(${CMAKE_SOURCE_DIR}/CMakeCommon/GroupSourcesMSVC.cmake)
GroupSourcesMSVC(include/embb/base)
GroupSourcesMSVC(src)
if (BUILD_TESTS STREQUAL ON)
  GroupSourcesMSVC(test)
endif()
if (BUILD_BENCHMARKS STREQUAL ON)
  GroupSourcesMSVC(perf)
endif()

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include
                    ${CMAKE_CURRENT_BINARY_DIR}/include
                    ${CMAKE_CURRENT_SOURCE_DIR}/../base_c/include
                    ${CMAKE_CURRENT_BINARY_DIR}/../base_c/include)

## BUILD TARGETS
#
add_library (embb_base_cpp ${EMBB_BASE_CPP_SOURCES} ${EMBB_BASE_CPP_HEADERS})
target_link_libraries(embb_base_cpp embb_base_c)
if (BUILD_TESTS STREQUAL ON)
  include_directories(test/
                      ${CMAKE_CURRENT_BINARY_DIR}/../partest/include
                      )
  add_executable (embb_base_cpp_test ${EMBB_BASE_TEST_SOURCES})
  target_link_libraries(embb_base_cpp_test
                        partest 
                        embb_base_cpp
                        embb_base_c
                        ${compiler_libs})
  CopyBin(BIN embb_base_cpp_test DEST ${local_install_dir})
endif()
if (BUILD_BENCHMARKS STREQUAL ON)
  include_directories(perf/include/
                      ${EMBB_BASE_CPP_PERF_PAPI_INC})
  add_library(embb_base_cpp_perf 
              ${EMBB_BASE_CPP_PERF_SOURCES} 
              ${EMBB_BASE_CPP_PERF_HEADERS})
  target_link_libraries(embb_base_cpp_perf 
                        ${EMBB_BASE_CPP_PERF_PAPI_LIB}
                        embb_base_cpp
                        embb_base_c
                        ${EMBB_BASE_CPP_PERF_PAPI_LIB}
                        ${compiler_libs}
                       )
endif()

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/embb
        DESTINATION include FILES_MATCHING PATTERN "*.h")
install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/include/embb
        DESTINATION include FILES_MATCHING PATTERN "*.h")
install(TARGETS embb_base_cpp DESTINATION lib)

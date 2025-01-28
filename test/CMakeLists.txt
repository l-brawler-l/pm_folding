cmake_minimum_required(VERSION 3.14)

project(xiTests LANGUAGES CXX)

include(../cmake/project-is-top-level.cmake)
include(../cmake/folders.cmake)

add_executable(xi_test)

# ---- Dependencies ----

if(PROJECT_IS_TOP_LEVEL)
  find_package(xi REQUIRED)
  enable_testing()
endif()


cmake_policy(SET CMP0135 NEW)

include(FetchContent)
FetchContent_Declare(
    googletest
    GIT_REPOSITORY "https://github.com/google/googletest.git"
    GIT_TAG "main"
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)
include(GoogleTest)

# ---- Tests ----
set(LDOPA_TEST_DATA_PATH "${CMAKE_SOURCE_DIR}/test/data")
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/ldopa/constants.h.in"
    "${CMAKE_CURRENT_BINARY_DIR}/ldopa/constants.h"
)
target_include_directories(xi_test SYSTEM
    PUBLIC
    "\$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/ldopa>" #?
)

set(sources 
    ldopa/eventlog/csvlog_test.cpp
    ldopa/eventlog/sqlitelog_test.cpp

    ldopa/graphs/bidigraph_1_test.cpp
    ldopa/graphs/bidigraph_test.cpp
    ldopa/graphs/bidigraph_2_test.cpp

    ldopa/ts/algos/obsolete1/eventlog_ts_grviz_exporter_1_test.cpp
    ldopa/ts/algos/obsolete1/eventlog_tsbuilder_1_test.cpp

    ldopa/ts/algos/evlog_ts_red_dotwriter_1_test.cpp
    ldopa/ts/algos/ts_simple_builder_1_test.cpp
    ldopa/ts/algos/ts_misc_converters_1_test.cpp

    ldopa/ts/models/evlog_ts_red_1_test.cpp
    ldopa/ts/models/labeledts_1_test.cpp
    ldopa/ts/models/eventlog_ts_2_test.cpp

    ldopa/ts/models/obsolete1/eventlog_ts_1_test.cpp
    ldopa/ts/models/obsolete1/basets_1_test.cpp

    ldopa/performance/performance_1_test.cpp
)
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${sources})

target_sources(xi_test PRIVATE ${sources})

# add_executable(xi_test ldopa/eventlog/csvlog_test.cpp)
target_link_libraries(xi_test PRIVATE 
    xi::xi
    GTest::gtest_main
)
target_compile_features(xi_test PRIVATE cxx_std_17)

# add_test(NAME xi_test COMMAND xi_test)
gtest_discover_tests(xi_test)

# ---- End-of-file commands ----

add_folders(Test)

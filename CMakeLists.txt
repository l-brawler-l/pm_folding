cmake_minimum_required(VERSION 3.15)

include(cmake/prelude.cmake)

project(
    xi
    VERSION 0.1.0
    DESCRIPTION "Library for process mining"
    LANGUAGES CXX
)

include(cmake/project-is-top-level.cmake)
include(cmake/variables.cmake)

# ---- Declare library ----

set(sources
    source/strutils/substr.cpp
    source/strutils/set_str_pool.cpp
    source/attributes/destructable_object.cpp
    # source/dllmain.cpp
    source/ldopa/ts/algos/varws_ts_rebuilder.cpp
    source/ldopa/ts/algos/ts_simple_builder.cpp
    source/ldopa/ts/algos/dual_ts_simulator.cpp
    source/ldopa/ts/algos/obsolete1/eventlogts_builder.cpp
    source/ldopa/ts/algos/obsolete1/eventlogts_grviz_exporter.cpp
    source/ldopa/ts/algos/freq_condenser.cpp
    source/ldopa/ts/algos/ts_metrics_calc.cpp
    source/ldopa/ts/algos/ts_folding_builder.cpp
    source/ldopa/ts/algos/cycle_condenser.cpp
    source/ldopa/ts/models/eventlog_ts_stateids.cpp
    source/ldopa/ts/models/evlog_ts_red.cpp
    source/ldopa/ts/models/evlog_ts_fold.cpp
    source/ldopa/ts/models/parikh_vector.cpp
    source/ldopa/ts/models/eventlog_ts.cpp
    source/ldopa/ts/models/obsolete1/eventlogts.cpp
    source/ldopa/ts/models/obsolete1/basets.cpp
    source/ldopa/utils.cpp
    source/ldopa/eventlog/sqlite/sqlitelog.cpp
    source/ldopa/eventlog/sqlite/sqlitehelpers.cpp
    source/ldopa/eventlog/obsolete1/csvlog.cpp
    source/ldopa/eventlog/obsolete1/csvlog_events.cpp
    # source/ldopa/dllmain.cpp
    source/ldopa/graphs/algos/grviz/bidigraph_dot_writer.cpp
    source/ldopa/graphs/algos/grviz/graph_dot_writer.cpp
    source/ldopa/graphs/bidigraph.cpp
    source/ldopa/graphs/_obsolete/named_bidigraph.cpp
    )

add_library(
    xi_xi
    ${sources}
)
add_library(xi::xi ALIAS xi_xi)


find_package(
    Boost REQUIRED COMPONENTS
    graph
)
if(NOT Boost_FOUND)
	message(SEND_ERROR "Failed to find boost.")
	return()
else()
	include_directories(${Boost_INCLUDE_DIRS})
    target_link_libraries(xi_xi 
        Boost::boost 
        Boost::graph
    )
endif()

include(cmake/FindSQLite3.cmake)

find_package (SQLite3 REQUIRED)
if(NOT SQLITE3_FOUND)
	message(SEND_ERROR "Failed to find sqlite.")
	return()
else()
    include_directories(${SQLITE3_INCLUDE_DIRS})
    target_link_libraries (xi_xi ${SQLITE3_LIBRARIES})
endif()



include(GenerateExportHeader)
generate_export_header(
    xi_xi
    BASE_NAME xi
    EXPORT_FILE_NAME export/xi/xi_export.hpp
    CUSTOM_CONTENT_FROM_VARIABLE pragma_suppress_c4251
)

if(NOT BUILD_SHARED_LIBS)
  target_compile_definitions(xi_xi PUBLIC XI_STATIC_DEFINE)
endif()

set_target_properties(
    xi_xi PROPERTIES
    CXX_VISIBILITY_PRESET hidden
    VISIBILITY_INLINES_HIDDEN YES
    VERSION "${PROJECT_VERSION}"
    SOVERSION "${PROJECT_VERSION_MAJOR}"
    EXPORT_NAME xi
    OUTPUT_NAME xi
)

target_include_directories(
    xi_xi ${warning_guard}
    PUBLIC
    "\$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>"
)

target_include_directories(
    xi_xi SYSTEM
    PUBLIC
    "\$<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/export>"
)

target_compile_features(xi_xi PUBLIC cxx_std_17)

# ---- Install rules ----

if(NOT CMAKE_SKIP_INSTALL_RULES)
  include(cmake/install-rules.cmake)
endif()

# ---- Developer mode ----

if(NOT xi_DEVELOPER_MODE)
  return()
elseif(NOT PROJECT_IS_TOP_LEVEL)
  message(
      AUTHOR_WARNING
      "Developer mode is intended for developers of xi"
  )
endif()

include(cmake/dev-mode.cmake)

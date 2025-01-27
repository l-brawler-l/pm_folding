# cmake_minimum_required(VERSION 3.14)

# project(xiTests LANGUAGES CXX)

# include(../cmake/project-is-top-level.cmake)
# include(../cmake/folders.cmake)

# # ---- Dependencies ----

# if(PROJECT_IS_TOP_LEVEL)
#   find_package(xi REQUIRED)
#   enable_testing()
# endif()

# # ---- Tests ----

# add_executable(xi_test source/xi_test.cpp)
# target_link_libraries(xi_test PRIVATE xi::xi)
# target_compile_features(xi_test PRIVATE cxx_std_17)

# add_test(NAME xi_test COMMAND xi_test)

# # ---- End-of-file commands ----

# add_folders(Test)

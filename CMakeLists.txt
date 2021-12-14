cmake_minimum_required(VERSION 3.1)

set(PROJ_NAME "projname")
set(PROJ_DESC "projdesc")

set(LIB_NAME "projlib")
set(EXEC_NAME "projexec")

set(TEST_TARGET "tests")

project(${PROJ_NAME} DESCRIPTION ${PROJ_DESC} LANGUAGES CXX)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_library(project_options INTERFACE)
target_compile_features(project_options INTERFACE cxx_std_20)

# compiler flags given as an interface
include(cmake/CompilerWarnings.cmake)
add_library(project_warnings INTERFACE)
set_project_warnings(project_warnings)

# Check that all git submodules are present
include(cmake/SubmodulesCheck.cmake)
check_submodules()

set(LIB_DIR "${CMAKE_CURRENT_SOURCE_DIR}/lib")
set(SOURCES_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")
set(TESTS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/tests")

# Gather files
file(GLOB_RECURSE SOURCE_FILES "${SOURCES_DIR}/*.cpp")
file(GLOB_RECURSE HEADER_FILES "${SOURCES_DIR}/*.h")
file(GLOB_RECURSE TEST_FILES "${TESTS_DIR}/*.cpp")

# BOOST
find_package(Boost REQUIRED COMPONENTS program_options filesystem)

# [LIBRARY] Source
add_library(${LIB_NAME} ${SOURCE_FILES} ${HEADER_FILES})
target_include_directories(${LIB_NAME} PUBLIC ${SOURCES_DIR} ${Boost_INCLUDE_DIRS})
set_target_properties(${LIB_NAME} PROPERTIES LINKER_LANGUAGE CXX)
target_link_libraries(${LIB_NAME} project_options project_warnings Boost::program_options Boost::filesystem)


# [EXECUTABLE] Main
add_executable(${EXEC_NAME} main.cpp)
target_link_libraries(${EXEC_NAME} PUBLIC ${LIB_NAME})

# [TESTS] Catch2
set(TEST_MAIN unit_tests)

add_subdirectory(lib/Catch2)
add_executable(${TEST_MAIN} ${TEST_FILES})
target_link_libraries(${TEST_MAIN} Catch2::Catch2WithMain ${LIB_NAME})

add_custom_target(${TEST_TARGET}
	COMMAND "${PROJECT_BINARY_DIR}/${TEST_MAIN}"
	DEPENDS ${TEST_MAIN}
)

include(CTest)
include(Catch)
catch_discover_tests(${TEST_MAIN})

execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink ${PROJECT_SOURCE_DIR}/res ${PROJECT_BINARY_DIR}/res RESULT_VARIABLE exitcode)

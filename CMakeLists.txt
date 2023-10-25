cmake_minimum_required(VERSION 3.20)

project(eva LANGUAGES CXX)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_C_STANDARD 11)
include(FetchContent)
FetchContent_Declare(googletest
	GIT_REPOSITORY https://github.com/google/googletest
	GIT_TAG v1.14.0)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

add_executable(${PROJECT_NAME} 
  main.cpp
  tokenizer.h
  parser.h
  NatvisFile.natvis 
)

target_link_libraries(${PROJECT_NAME} GTest::gtest)

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY 
	VS_STARTUP_PROJECT ${PROJECT_NAME})

set_target_properties(${PROJECT_NAME} PROPERTIES
	VS_DEBUGGER_WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/test_programs)
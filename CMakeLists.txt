cmake_minimum_required(VERSION 3.23)

project(
    batchelor
    VERSION 1.0.0
    LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)

include(FetchContent)

message(STATUS "Try to find SerGut by FetchContent")
FetchContent_Declare(
    sergut
    GIT_REPOSITORY https://github.com/slukasde/sergut
    GIT_TAG        master
    GIT_SHALLOW    TRUE
    OVERRIDE_FIND_PACKAGE # 'find_package(...)' will call 'FetchContent_MakeAvailable(...)'
)

message(STATUS "Try to find OpenESL by FetchContent")
#set(OPENESL_USE_COMMON4ESL OFF)
FetchContent_Declare(
    openesl
    GIT_REPOSITORY https://github.com/slukasde/OpenESL
    #GIT_TAG        v1.6.0
    GIT_SHALLOW    TRUE
    OVERRIDE_FIND_PACKAGE # 'find_package(...)' will call 'FetchContent_MakeAvailable(...)'
)

add_subdirectory(batchelor-common)
add_subdirectory(batchelor-service)
add_subdirectory(batchelor-control)
add_subdirectory(batchelor-condition)
add_subdirectory(batchelor-head)
add_subdirectory(batchelor-ui)
add_subdirectory(batchelor-worker)


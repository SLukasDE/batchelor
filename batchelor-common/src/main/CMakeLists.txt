file(GLOB_RECURSE SRC ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
add_library(batchelor-common STATIC ${SRC})
target_include_directories(batchelor-common PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

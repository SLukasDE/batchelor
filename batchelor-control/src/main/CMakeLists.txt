file(GLOB_RECURSE SRC ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
add_library(batchelor-control STATIC ${SRC})
target_include_directories(batchelor-control PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

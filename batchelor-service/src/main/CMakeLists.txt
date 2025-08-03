file(GLOB_RECURSE SRC ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
add_library(batchelor-service STATIC ${SRC})
target_include_directories(batchelor-service PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

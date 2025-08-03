file(GLOB_RECURSE SRC ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
add_executable(batchelor-worker ${SRC})
target_include_directories(batchelor-worker PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

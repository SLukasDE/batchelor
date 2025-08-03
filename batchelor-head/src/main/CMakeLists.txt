file(GLOB_RECURSE SRC ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
add_executable(batchelor-head ${SRC})
target_include_directories(batchelor-head PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

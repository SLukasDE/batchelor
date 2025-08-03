file(GLOB_RECURSE SRC ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
add_executable(batchelor-ui ${SRC})
target_include_directories(batchelor-ui PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

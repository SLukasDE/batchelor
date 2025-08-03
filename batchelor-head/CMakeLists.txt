add_subdirectory(src/main)

target_link_libraries(batchelor-head PUBLIC
    batchelor-service
    batchelor-common
    batchelor-condition)

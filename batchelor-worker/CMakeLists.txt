add_subdirectory(src/main)

target_link_libraries(batchelor-worker PUBLIC
    batchelor-service
    batchelor-common)

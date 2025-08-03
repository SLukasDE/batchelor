add_subdirectory(src/main)

target_link_libraries(batchelor-ui PUBLIC
    batchelor-service
    batchelor-common)

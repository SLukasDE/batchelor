add_subdirectory(src/main)

add_library(batchelor-control::batchelor-control ALIAS batchelor-control)

target_link_libraries(batchelor-control PUBLIC
    batchelor-common
    batchelor-service)

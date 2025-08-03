find_package(sergut REQUIRED)
find_package(openesl REQUIRED)

add_subdirectory(src/main)

add_library(batchelor-service::batchelor-service ALIAS batchelor-service)

target_link_libraries(batchelor-service PUBLIC
    sergut::sergut
    openesl::openesl)

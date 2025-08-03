find_package(tinyxml2 REQUIRED)
find_package(openesl REQUIRED)
find_package(BISON REQUIRED)
find_package(FLEX REQUIRED)

add_subdirectory(src/main)

add_library(batchelor-condition::batchelor-condition ALIAS batchelor-condition)

target_link_libraries(batchelor-condition PUBLIC
    tinyxml2::tinyxml2
    openesl::openesl)

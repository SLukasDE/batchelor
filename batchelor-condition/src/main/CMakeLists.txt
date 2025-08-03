file(GLOB_RECURSE SRC ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)

add_library(batchelor-condition STATIC ${SRC} ${CMAKE_BINARY_DIR}/batchelor-condition/src/main/batchelor/condition/Scanner.cpp ${CMAKE_BINARY_DIR}/batchelor-condition/src/main/batchelor/condition/Parser.cpp)
target_include_directories(batchelor-condition PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_BINARY_DIR}/batchelor-condition/src/main)
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/batchelor-condition/src/main/batchelor/condition)

add_custom_command(
  SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/batchelor/condition/Scanner.l
  COMMAND ${FLEX_EXECUTABLE}
  ARGS --outfile=${CMAKE_BINARY_DIR}/batchelor-condition/src/main/batchelor/condition/Scanner.cpp ${CMAKE_CURRENT_SOURCE_DIR}/batchelor/condition/Scanner.l
  COMMENT "Generating Lexer.cpp"
  OUTPUT ${CMAKE_BINARY_DIR}/batchelor-condition/src/main/batchelor/condition/Scanner.cpp
)

add_custom_command(
  SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/batchelor/condition/Parser.yy
  COMMAND ${BISON_EXECUTABLE}
  ARGS -L C++ -d -v ${CMAKE_CURRENT_SOURCE_DIR}/batchelor/condition/Parser.yy
  COMMENT "Generating Parser.cpp"
  OUTPUT ${CMAKE_BINARY_DIR}/batchelor-condition/src/main/batchelor/condition/Parser.cpp
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/batchelor-condition/src/main/batchelor/condition/
)

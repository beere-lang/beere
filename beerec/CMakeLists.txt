cmake_minimum_required(VERSION 3.31)
project(beerec C)

set(CMAKE_C_STANDARD 99)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

add_executable(beerec
      src/init/main.c
      src/init/compiler.c
      src/frontend/lexer/lexer.c
      src/utils/utils.c
      src/utils/logger/logger.c
      src/utils/string/string.c
      src/utils/file/file.c
      src/utils/list/list.c
	src/frontend/parser/parser.c
	src/frontend/semantic/analyzer.c
      src/frontend/modules/modules.c
      #src/backend/ir/ir-gen.c
      #src/backend/structure/ir/ir-nodes.c
)
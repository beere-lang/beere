cmake_minimum_required(VERSION 3.31)
project(beerec C)

set(CMAKE_C_STANDARD 99)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

add_executable(beerec
        src/init/main.c
        src/init/compiler.c
        src/frontend/lexer/lexer.c
        src/frontend/structure/parser/parser.c
        src/frontend/semantic/analyzer/analyzer.c
        src/utils/utils.c
        src/utils/logger/logger.c
        src/utils/string_builder/string_builder.c
        src/frontend/modules/modules.c
        src/frontend/modules/handler/module-handler.c
)
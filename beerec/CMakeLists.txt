cmake_minimum_required(VERSION 3.31)
project(beerec C)

set(CMAKE_C_STANDARD 99)

add_compile_options(-fstack-usage)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--stack,8388608")

add_executable(beerec
        compiler/main.c
		compiler/compiler.c
        src/lexer/lexer.c
        src/parser/parser.c
        src/analyzer/analyzer.c
        src/utils/utils.c
        src/utils/logger/logger.c
        src/utils/string_builder/string_builder.c
		src/modules/modules.c
		src/modules/handler/module-handler.c
	src/codegen/code-gen.c
)
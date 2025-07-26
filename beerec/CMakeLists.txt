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
		src/codegen/codegen.c
		src/codegen/impl/expressions/codegen-expr.c
		src/codegen/impl/expressions/arithmetic/codegen-arith.c
		src/codegen/impl/expressions/literals/codegen-lit.c
		src/codegen/impl/statements/calls/codegen-method-call.c
		src/codegen/impl/expressions/operations/codegen-op.c
		src/codegen/impl/expressions/references/codegen-ref.c
		src/codegen/impl/expressions/cast/codegen-cast.c
		src/codegen/impl/statements/declaration/fields/codegen-field-decl.c
		src/codegen/impl/statements/declaration/methods/codegen-method-decl.c
		src/codegen/impl/statements/assign/fields/codegen-field-assgn.c
		src/codegen/impl/statements/return/codegen-return.c
)
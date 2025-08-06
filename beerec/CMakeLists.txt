cmake_minimum_required(VERSION 3.31)
project(beerec C)

set(CMAKE_C_STANDARD 99)

add_compile_options(-fstack-usage)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--stack,8388608")

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
	src/backend/codegen/codegen.c
	src/backend/codegen/impl/expressions/codegen-expr.c
	src/backend/codegen/impl/oop/codegen-class.c
	src/backend/codegen/impl/oop/instances/codegen-create-instance.c
	src/backend/codegen/impl/oop/member-access/codegen-mbr-access.c
	src/backend/codegen/impl/expressions/arithmetic/codegen-arith.c
	src/backend/codegen/impl/expressions/literals/codegen-lit.c
	src/backend/codegen/impl/statements/calls/codegen-method-call.c
	src/backend/codegen/impl/expressions/operations/codegen-op.c
	src/backend/codegen/impl/expressions/references/codegen-ref.c
	src/backend/codegen/impl/expressions/pointers/codegen-ptr.c
	src/backend/codegen/impl/expressions/cast/codegen-cast.c
	src/backend/codegen/impl/expressions/comparative/codegen-comparative.c
	src/backend/codegen/impl/statements/declaration/fields/codegen-field-decl.c
	src/backend/codegen/impl/statements/declaration/methods/codegen-method-decl.c
	src/backend/codegen/impl/statements/assign/fields/codegen-field-assgn.c
	src/backend/codegen/impl/statements/return/codegen-return.c
	src/backend/codegen/impl/statements/if/codegen-if.c
	src/backend/codegen/impl/statements/while-loop/codegen-while.c
)
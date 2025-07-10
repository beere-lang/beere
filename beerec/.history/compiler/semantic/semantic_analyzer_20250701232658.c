#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "semantic_analyzer.h"
#include "../parser/parser.h"

SymbolTable global_scope;

/**
 * TODO:
 * - Implementar suporte a `super` (acesso/chamada de métodos/campos da superclasse).
 * - Implementar métodos virtuais e override (incluindo checagem de assinatura e vtable, se necessário).
 * - Implementar e validar visibilidade (public/private/protected) para campos e métodos.
 * - Finalizar suporte a strings imutáveis em todas as fases (lexer, parser, analyzer, codegen).
 * - Garantir diferenciação entre métodos de instância e estáticos, e uso correto de `this`.
 * - Implementar/descrever destrutores automáticos para objetos e strings.
 * - Testar todos os cenários de OOP, strings, shadowing, override, visibilidade, polimorfismo, etc.
 * - Planejar e documentar o layout de objetos/classes para o runtime e codegen.
 * - Implementar cast explicito pra objetos.
 * 
 * Pontos de atenção:
 * - Herança de métodos e campos, incluindo shadowing e warnings.
 * - Checagem de compatibilidade de tipos de classe (herança, ponteiros, utilitário).
 * - Parsing e semântica de `this`, `super`.
 * - Mensagens de erro claras para todos os casos.
 * - Literais de string: área estática, concatenação, comparação, métodos.
 * - Gerenciamento manual de memória para objetos e strings.
 *
 * - IMPORTANT: Adicionar e implementar arrays.
 */

static Node* get_function_from_class(Symbol* class, char* func_name);
static Node* get_member_from_class(Symbol* class, char* member_name);

void analyze_node(Node* node, SymbolTable* scope, int* offset);
int is_type_identic(Type* first, Type* second);
void create_cast(Node** node, Type* preferred);

void setup_scope(SymbolTable* scope, SymbolType scope_kind, SymbolTable* parent, Symbol* owner_statement)
{
    scope->scope_kind = scope_kind;
    scope->count = 0;
    scope->parent = parent;
    scope->owner_statement = owner_statement;

    scope->symbols = malloc(sizeof(Symbol*) * 4);
    
    if (scope->symbols == NULL)
    {
        printf("[Analyzer] [Debug] Failed to allocate memory for symbols global scope array...\n");
        exit(1);
    }

    scope->capacity = 4;
    scope->symbols[0] = NULL;
}

void init_analyzer()
{
    setup_scope(&global_scope, GLOBAL_SCOPE, NULL, NULL);
}

void global_analyze(Node* node)
{
    analyze_node(node, &global_scope, NULL);
}

/**
 * TODO: Implementar classes nisso, pegando o tamanho de todos as fields recursivamente.
 */
int get_type_size(Type* type)
{
    switch (type->type) 
    {
        case TYPE_BOOL:
        {
            return 4;
        }
        case TYPE_INT:
        {
            return 4;
        }
        case TYPE_FLOAT:
        {
            return 4;
        }
        case TYPE_DOUBLE:
        {
            return 8;
        }
        case TYPE_STRING:
        {
            return 8;
        }
        case TYPE_PTR:
        {
            return 8;
        }
        case TYPE_ANY_PTR:
        {
            return 8;
        }
        case TYPE_CHAR:
        {
            return 1;
        }
        case TYPE_NULL:
        {
            return 8;
        }
        default:
        {
            printf("[Analyzer] [Debug] Invalid type...\n");
            exit(1);
        }
    }

    exit(1);
}

int is_numeric(Type* type)
{
    if (type->type == TYPE_DOUBLE)
    {
        return 1;
    }
    if (type->type == TYPE_FLOAT)
    {
        return 1;
    }
    if (type->type == TYPE_INT)
    {
        return 1;
    }

    return 0;
}

SymbolTable* create_scope(SymbolType scope_kind, SymbolTable* parent, Symbol* owner_statement)
{
    SymbolTable* scope = malloc(sizeof(SymbolTable));
    setup_scope(scope, scope_kind, parent, owner_statement);

    return scope;
}

void _add_symbol_to_scope(Symbol* symbol, SymbolTable* scope)
{
    if (symbol == NULL)
    {
        printf("[Analyzer] [Debug] Symbol param is null...\n");
        exit(1);
    }
    if (scope == NULL)
    {
        printf("[Analyzer] [Debug] Scope param is null...\n");
        exit(1);
    }
    
    if (scope->capacity <= scope->count)
    {
        scope->capacity *= 2;
        
        scope->symbols = realloc(scope->symbols, sizeof(Symbol*) * scope->capacity);

        if (scope->symbols == NULL)
        {
            printf("[Analyzer] [Debug] Failed to realloc memory for scope's symbols array...\n");
            exit(1);
        }
    }

    scope->symbols[scope->count] = symbol;
    scope->count++;
    scope->symbols[scope->count] = NULL;
}

Symbol* create_symbol(SymbolType type, Node* node, SymbolTable* scope, int param, int* offset)
{
    if (type == SYMBOL_VARIABLE && !param)
    {
        Symbol* symbol = malloc(sizeof(Symbol));
        symbol->type = SYMBOL_VARIABLE;

        symbol->symbol_variable = malloc(sizeof(SymbolVariable));
        
        symbol->symbol_variable->is_global = (scope->scope_kind != SYMBOL_FUNCTION);

        symbol->symbol_variable->type = node->declare_node.declare.var_type;
        symbol->symbol_variable->identifier = node->declare_node.declare.identifier;
        symbol->symbol_variable->is_const = node->declare_node.declare.imut;

        if (offset != NULL)
        {
            if (symbol->symbol_variable->is_global)
            {
                printf("[Analyzer] [Debug] Found a global variable with a valid offset: %d...\n", *offset);
                exit(1);
            }
            printf("[Analyzer] [Debug] Added a local variable with offset: %d...\n", *offset);

            symbol->symbol_variable->offset = *offset;
        }
        else if (offset == NULL && !symbol->symbol_variable->is_global)
        {
            printf("[Analyzer] [Debug] Found a local variable with a invalid offset...\n");
            exit(1);
        }

        _add_symbol_to_scope(symbol, scope);

        return symbol;
    }
    if (type == SYMBOL_VARIABLE && param)
    {
        Symbol* symbol = malloc(sizeof(Symbol));
        symbol->type = SYMBOL_VARIABLE;

        symbol->symbol_variable = malloc(sizeof(SymbolVariable));

        symbol->symbol_variable->type = node->param_node.param.argument_type;
        symbol->symbol_variable->identifier = node->param_node.param.identifier;

        symbol->symbol_variable->is_const = 0;
        
        if (offset != NULL)
        {
            printf("[Analyzer] [Debug] Added a param: \"%s\" with offset: %d...\n", symbol->symbol_variable->identifier, *offset);
        }
        else
        {
            printf("[Analyzer] [Debug] Found a param with a invalid offset...\n");
            exit(1);
        }
        
        symbol->symbol_variable->offset = *offset;
        
        symbol->symbol_variable->is_global = 0;
        
        _add_symbol_to_scope(symbol, scope);

        return symbol;
    }
    if (type == SYMBOL_FUNCTION)
    {
        Symbol* symbol = malloc(sizeof(Symbol));
        symbol->type = SYMBOL_FUNCTION;

        symbol->symbol_function = malloc(sizeof(SymbolFunction));

        symbol->symbol_function->return_type = node->function_node.function.return_type;
        symbol->symbol_function->identifier = node->function_node.function.identifier;

        if (node->function_node.function.params != NULL)
        {
            symbol->symbol_function->params_head = node->function_node.function.params->head;
        }
        else
        {
            symbol->symbol_function->params_head = NULL;
        }
        
        _add_symbol_to_scope(symbol, scope);

        return symbol;
    }
    if (type == SYMBOL_CLASS)
    {
        Symbol* symbol = malloc(sizeof(Symbol));
        symbol->type = SYMBOL_CLASS;

        symbol->symbol_class = malloc(sizeof(SymbolClass));

        symbol->symbol_class->class_scope = NULL;

        symbol->symbol_class->identifier = node->class_node.class_node.identifer;

        symbol->symbol_class->functions = node->class_node.class_node.func_declare_list;
        symbol->symbol_class->fields = node->class_node.class_node.var_declare_list;

        symbol->symbol_class->func_count = node->class_node.class_node.func_count;
        symbol->symbol_class->field_count = node->class_node.class_node.var_count;
        
        _add_symbol_to_scope(symbol, scope);

        return symbol;
    }

    return NULL;
}

Symbol* add_symbol_to_scope(Node* node, SymbolTable* scope, int* offset)
{
    switch (node->type) 
    {
        case NODE_DECLARATION:
        {
            return create_symbol(SYMBOL_VARIABLE, node, scope, 0, offset);

            break;
        }
        case NODE_FUNCTION:
        {
            return create_symbol(SYMBOL_FUNCTION, node, scope, 0, offset);
            
            break;
        }
        case NODE_PARAMTER:
        {
            return create_symbol(SYMBOL_VARIABLE, node, scope, 1, offset);
            
            break;
        }
        default:
        {
            printf("[Analyzer] [Debug] Node type not valid for a symbol...\n");
            exit(1);
        }
    }

    return NULL;
}

Symbol* find_symbol_from_scope(const char* identifier, SymbolTable* scope, int is_function, int is_class)
{
    if (scope == NULL) 
    {
        return NULL;
    }

    int i = 0;

    Symbol* next = scope->symbols[i];

    while (next != NULL)
    {
        if (next->type == SYMBOL_FUNCTION && is_function) 
        {
            if (strcmp(identifier, next->symbol_function->identifier) == 0)
            {
                return next;
            }
        }
        else if (next->type == SYMBOL_VARIABLE && !is_function) 
        {
            if (strcmp(identifier, next->symbol_function->identifier) == 0)
            {
                return next;
            }
        }
        else if (next->type == SYMBOL_CLASS && is_class) 
        {
            if (strcmp(identifier, next->symbol_class->identifier) == 0)
            {
                return next;
            }
        }

        next = scope->symbols[i];
        i++;
    }

    return find_symbol_from_scope(identifier, scope->parent, is_function, is_class);
}

int is_equals(Type* type, VarType enum_type)
{
    if (type->type == enum_type)
    {
        return 1;
    }

    return 0;
}

/**
 * TODO: Adicionar suporte pra ponteiros que tem classes que são assignable.
 */
int is_compatible(Type* first, Type* second)
{
    if (is_numeric(first) && is_numeric(second))
    {
        return 1;
    }
    if (is_equals(first, TYPE_BOOL) && is_equals(second, TYPE_BOOL))
    {
        return 1;
    }
    if (is_equals(first, TYPE_STRING) && is_equals(second, TYPE_STRING))
    {
        return 1;
    }
    if (is_equals(first, TYPE_CHAR) && is_equals(second, TYPE_CHAR))
    {
        return 1;
    }
    if (is_equals(first, TYPE_PTR) || is_equals(second, TYPE_PTR))
    {
        if (is_type_identic(first, second))
        {
            return 1;
        }
        else 
        {
            printf("[Analyzer] [Debug] Pointers type aren't the same...\n");
        }
    }

    return 0;
}

Type* get_higher(Type* first, Type* second)
{
    if (is_equals(first, TYPE_DOUBLE) || is_equals(second, TYPE_DOUBLE)) 
    {
        return create_type(TYPE_DOUBLE, NULL);
    }
    if (is_equals(first, TYPE_FLOAT) || is_equals(second, TYPE_FLOAT)) 
    {
        return create_type(TYPE_FLOAT, NULL);
    }
    if (is_equals(first, TYPE_INT) || is_equals(second, TYPE_INT)) 
    {
        return create_type(TYPE_INT, NULL);
    }

    return first;
}

SymbolTable* get_class_scope(SymbolTable* scope)
{
    if (scope == NULL)
    {
        return NULL;
    }
    if (scope->scope_kind == SYMBOL_CLASS)
    {
        return scope;
    }

    return scope->parent;
}

/**
 * TODO: Implementar suporte a function calls.
 *
 * Functions calls provavelmente não vão funcionar porque:
 * - Callee usa member access, exemplo: object->function() --> callee: FunctionCallNode: { callee: MemberAccessNode: { object: object } }
 */
Type* type_of_expression(Node* expression, SymbolTable* scope)
{
    if (expression == NULL)
    {
        printf("[Analyzer] [Debug] Expression is null...\n");
        exit(1);
    }

    switch (expression->type) 
    {
        case NODE_OPERATION:
        {
            Type* left = type_of_expression(expression->operation_node.operation.left, scope);
            Type* right = type_of_expression(expression->operation_node.operation.right, scope);

            switch (expression->operation_node.operation.op)
            {
                case TOKEN_OPERATOR_OR: // ||
                case TOKEN_OPERATOR_AND: // &&
                {
                    return create_type(TYPE_BOOL, NULL);
                }
                
                case TOKEN_OPERATOR_DIVIDED_EQUALS: // /=
                case TOKEN_OPERATOR_TIMES_EQUALS: // *=
                case TOKEN_OPERATOR_MINUS_EQUALS: // -=
                case TOKEN_OPERATOR_PLUS_EQUALS: // +=
                case TOKEN_OPERATOR_DIVIDED: // /
                case TOKEN_CHAR_STAR: // "*" multiply
                case TOKEN_OPERATOR_MINUS: // -
                case TOKEN_OPERATOR_PLUS: // +
                case TOKEN_OPERATOR_DECREMENT: // --
                case TOKEN_OPERATOR_INCREMENT: // ++
                {
                    return get_higher(left, right);
                }
                
                case TOKEN_OPERATOR_LESS: // <
                case TOKEN_OPERATOR_LESS_EQUALS: // <=
                case TOKEN_OPERATOR_GREATER_EQUALS: // >=
                case TOKEN_OPERATOR_GREATER: // >
                case TOKEN_OPERATOR_EQUALS: // ==
                {
                    return create_type(TYPE_BOOL, NULL);
                }
                default:
                {
                    printf("[Analyzer] [Debug] Invalid operation operator (type_of_expression)...\n");
                    exit(1);
                }
            }
        }
        case NODE_LITERAL:
        {
            return expression->literal_node.literal_node.literal_type;
        }
        case NODE_IDENTIFIER:
        {
            Symbol* symbol = find_symbol_from_scope(expression->variable_node.variable.identifier, scope, 0, 0);
            
            if (symbol == NULL)
            {
                printf("[Analyzer] [Debug] Variable not declared: %s...\n", expression->variable_node.variable.identifier);
                exit(1);
            }

            return symbol->symbol_variable->type;
        }
        /**
         * FIXME: Terminar e implementar isso.
         */
        case NODE_FUNCTION_CALL:
        {
            
        }
        case NODE_CAST:
        {
            return expression->cast_statement_node.cast_node.cast_type;
        }
        case NODE_ADRESS_OF:
        {
            Type* inner = type_of_expression(expression->adress_of_node.adress_of.expression, scope);

            Type* ptr = malloc(sizeof(Type));
            ptr->type = TYPE_PTR;
            ptr->base = inner;

            return ptr;
        }
        case NODE_DEREFERENCE:
        {
            Type* type = NULL;
            Node* ref = expression;
            Node* _ref = expression;

            while (_ref->type == NODE_DEREFERENCE)
            {
                _ref = _ref->dereference_node.dereference.ptr;
            }

            if (_ref->type == NODE_IDENTIFIER)
            {
                type = type_of_expression(_ref, scope);
            }
            
            int depth = 0;

            while (ref->type == NODE_DEREFERENCE)
            {
                ref = ref->dereference_node.dereference.ptr;
                depth++;
            }

            for (int i = 0; i < depth; i++)
            {
                if (type->base == NULL)
                {
                    printf("[Analyzer] [Debug] Deferencing more than the pointer's depth...\n");
                    exit(1);
                }

                type = type->base;
            }

            return type;
        }
        case NODE_DECLARATION:
        {
            return expression->declare_node.declare.var_type;
        }
        case NODE_CREATE_INSTANCE:
        {
            Type* type = create_type(TYPE_PTR, NULL);
            type->base = create_type(TYPE_CLASS, expression->create_instance_node.create_instance.class_name);

            return type;
        }
        case NODE_THIS:
        {
            Type* type = create_type(TYPE_PTR, NULL);
            type->base = create_type(TYPE_CLASS, (char*) get_class_scope(scope)->owner_statement->symbol_class->identifier);

            return type;
        }
        /**
         * FIXME: Refatorar esse codigo todo, ta desorganizado.
         */
        case NODE_MEMBER_ACCESS:
        {
            Type* object_type = type_of_expression(expression->member_access_node.member_access.object, scope);

            if (object_type->type == TYPE_PTR && object_type->base->type == TYPE_CLASS)
            {
                if (!expression->member_access_node.member_access.ptr_acess)
                {
                    printf("[Analyzer] [Debug] Acessing a pointer with a '.'... (Use a '->' operator)\n");
                    exit(1);
                }
            }
            else if (object_type->type == TYPE_CLASS)
            {
                if (expression->member_access_node.member_access.ptr_acess)
                {
                    printf("[Analyzer] [Debug] Acessing a normal object with a '->'... (Use a '.' operator)\n");
                    exit(1);
                }
            }
            else 
            {
                printf("[Analyzer] [Debug] Object needs to be a class or a pointer to a class...\n");
                exit(1);
            }

            Symbol* class = NULL;

            if (expression->member_access_node.member_access.object->type == NODE_THIS)
            {
                class = find_symbol_from_scope(expression->member_access_node.member_access.ptr_acess ? object_type->base->class_name : object_type->class_name, get_class_scope(scope), 0, 1);
            }
            else 
            {
                class = find_symbol_from_scope(expression->member_access_node.member_access.ptr_acess ? object_type->base->class_name : object_type->class_name, scope, 0, 1);
            }

            if (class == NULL)
            {
                printf("[Analyzer] [Debug] Trying to acess a non declared class...\n");
                exit(1);
            }

            Node* member = get_member_from_class(class, expression->member_access_node.member_access.member_name);
            
            if (member == NULL)
            {
                printf("[Analyzer] [Debug] Field from class not found: \"%s\"...\n", expression->member_access_node.member_access.member_name);
                exit(1);
            }
            else 
            {
                if (member->declare_node.declare.visibility == VISIBILITY_PRIVATE)
                {
                    printf("[Analyzer] [Debug] Field from class is private: \"%s\"...\n", member->declare_node.declare.identifier);
                    exit(1);
                }

                printf("[Analyzer] [Debug] Field from class acessed: \"%s\"...\n", member->declare_node.declare.identifier);
            }

            return type_of_expression(member, class->symbol_class->class_scope);
        }
        default:
        {
            printf("[Analyzer] [Debug] Invalid expression type: %d...\n", expression->type);
            exit(1);
        }
    }

    return NULL;
}

int is_operation_compatible(Node* node, SymbolTable* scope)
{
    OperationNode* operation = &node->operation_node.operation;

    Type* left = type_of_expression(operation->left, scope);
    Type* right = type_of_expression(operation->right, scope);

    switch (operation->op) 
    {
        /**
         * Operadores de contexto.
         */
        case TOKEN_OPERATOR_OR: // ||
        case TOKEN_OPERATOR_AND: // &&
        {
            if (is_equals(left, TYPE_BOOL) && is_equals(right, TYPE_BOOL))
            {
                return 1;
            }

            break;
        }
        
        /**
         * Operadores aritmeticos.
         */
        case TOKEN_OPERATOR_DIVIDED_EQUALS: // /=
        case TOKEN_OPERATOR_TIMES_EQUALS: // *=
        case TOKEN_OPERATOR_MINUS_EQUALS: // -=
        case TOKEN_OPERATOR_PLUS_EQUALS: // +=
        case TOKEN_OPERATOR_DIVIDED: // /
        case TOKEN_CHAR_STAR: // "*" multiply
        case TOKEN_OPERATOR_MINUS: // -
        case TOKEN_OPERATOR_PLUS: // +
        case TOKEN_OPERATOR_INCREMENT: // ++
        case TOKEN_OPERATOR_DECREMENT: // --
        {
            if (is_compatible(left, right))
            {
                return 1;
            }
        }
        
        /**
         * Operadores comparativos.
         */
        case TOKEN_OPERATOR_LESS: // <
        case TOKEN_OPERATOR_LESS_EQUALS: // <=
        case TOKEN_OPERATOR_GREATER_EQUALS: // >=
        case TOKEN_OPERATOR_GREATER: // >
        case TOKEN_OPERATOR_EQUALS: // ==
        {
            if (is_compatible(left, right))
            {
                return 1;
            }

            break;
        }
        default:
        {
            printf("[Analyzer] [Debug] Invalid operation operator...\n");
            exit(1);
        }
    }

    return 0;
}

void implictly_cast_all(Type* preffered, Node* expression, SymbolTable* scope)
{
    Type* type = type_of_expression(expression, scope);

    if (type->type != TYPE_PTR && preffered->type != TYPE_PTR)
    {
        if (type->type == preffered->type)
        {
            printf("[Analyzer] [Debug] Not casting, types are the same...\n");
            return;
        }
    }

    if (!is_compatible(type, preffered))
    {
        printf("[Analyzer] [Debug] Expression type is incompatible...\n");
        exit(1);
    }

    create_cast(&expression, preffered);
}

/**
 * TODO: Implementar isso em declarações de funções.
 */
void analyze_type(Type* type, SymbolTable* scope)
{
    if (type == NULL)
    {
        return;
    }
    
    if (type->type == TYPE_CLASS)
    {
        if (type->class_name == NULL)
        {
            exit(1);
        }
        
        if (find_symbol_from_scope(type->class_name, scope, 0, 1) == NULL)
        {
            printf("[Analyzer] [Debug] Class of type: \"%s\" not found...\n", type->class_name);
            exit(1);
        }
    }

    analyze_type(type->base, scope);
}

/**
 * TODO: Adicionar suporte pra classes no metodo "get_type_size()".
 */
void handle_variable_declaration(Node* node, SymbolTable* scope, int* offset)
{ 
    if (find_symbol_from_scope(node->declare_node.declare.identifier, scope, 0, 0) != NULL)
    {
        printf("[Analyzer] [Debug] Variable already declared in scope...\n");
        exit(1);
    }

    analyze_type(node->declare_node.declare.var_type, scope);

    if (scope->scope_kind == SYMBOL_FUNCTION)
    {
        int size = get_type_size(node->declare_node.declare.var_type);
        *offset -= size;
    }

    add_symbol_to_scope(node, scope, offset);
    
    Node* expression = node->declare_node.declare.default_value;

    if (expression != NULL)
    {
        analyze_node(expression, scope, NULL);

        implictly_cast_all(node->declare_node.declare.var_type, expression, scope);
    }
}

void handle_paramters(Node* head, SymbolTable* scope, int* offset)
{
    Node* next = head;

    while (next != NULL)
    {
        int size = get_type_size(next->param_node.param.argument_type);

        *offset += size;

        if (find_symbol_from_scope(next->param_node.param.identifier, scope, 0, 0) != NULL)
        {
            printf("[Analyzer] [Debug] Parameter with name: \"%s\" already declared...\n", next->param_node.param.identifier);
            exit(1);
        }

        add_symbol_to_scope(next, scope, offset);

        next = next->next;
    }
}

void handle_function_declaration(Node* node, SymbolTable* scope)
{
    const FunctionNode* function_node = &node->function_node.function;
    
    if (find_symbol_from_scope(function_node->identifier, scope, 1, 0) != NULL)
    {
        printf("[Analyzer] [Debug] Function with name: \"%s\" already declared...\n", function_node->identifier);
        exit(1);
    }

    if (function_node->constructor)
    {
        if (strcmp(scope->owner_statement->symbol_class->identifier, function_node->identifier) != 0)
        {
            printf("[Analyzer] [Debug] Invalid constructor name, use the same name from class...\n");
            exit(1);
        }
    }

    int local_offset = 0;
    int param_offset = 8;

    Symbol* func_symbol = add_symbol_to_scope(node, scope, NULL);

    SymbolTable* block_scope = create_scope(SYMBOL_FUNCTION, scope, func_symbol);

    if (function_node->params != NULL)
    {
        handle_paramters(function_node->params->head, block_scope, &param_offset);
    }

    analyze_node(function_node->block_node, block_scope, &local_offset);
}

/**
 * TODO: Adicionar suporte a casts explicitos no parser e analizar se o cast pode ser feito.
 */
void create_cast(Node** node, Type* preferred)
{
    Node* cast = malloc(sizeof(Node));

    if (cast == NULL)
    {
        printf("[Analyzer] [Debug] Failed to alllocate memory for cast node...\n");
        exit(1);
    }

    cast->type = NODE_CAST;
    cast->cast_statement_node.cast_node.cast_type = preferred;
    cast->cast_statement_node.cast_node.expression = *node;

    *node = cast;
}

/**
 * TODO: 
 * - Adicionar suporte a strings.
 * - Adicionar suporte a classes caso necessario.
 */
void implictly_cast_operation(Node* node, SymbolTable* scope)
{
    OperationNode* operation = &node->operation_node.operation;

    Type* left = type_of_expression(operation->left, scope);
    Type* right = type_of_expression(operation->right, scope);

    Type* higher = get_higher(left, right);

    if (left != higher)
    {
        create_cast(&operation->left, higher);
    }
    if (right != higher)
    {
        create_cast(&operation->left, higher);
    }
}

void handle_operation(Node* node, SymbolTable* scope)
{
    OperationNode* operation = &node->operation_node.operation;
    
    if (!is_operation_compatible(node, scope))
    {
        printf("[Analyzer] [Debug] Incompatible operation...\n");
        exit(1);
    }

    analyze_node(operation->left, scope, NULL);
    analyze_node(operation->right, scope, NULL);

    implictly_cast_operation(node, scope);
}

void check_global_scope(Node* node, SymbolTable* scope)
{
    const NodeType type = node->type;
    const SymbolType kind = scope->scope_kind;
    
    if (type == NODE_FUNCTION && kind != GLOBAL_SCOPE && kind != SYMBOL_CLASS)
    {
        printf("[Analyzer] [Debug] Function declaration statement isn't on global scope...\n");
        exit(1);
    }
    if (type == NODE_IMPORT && kind != GLOBAL_SCOPE)
    {
        printf("[Analyzer] [Debug] Function declaration statement isn't on global scope...\n");
        exit(1);
    }
    if (type != NODE_FUNCTION && type != NODE_CLASS && type != NODE_DECLARATION && type != NODE_IMPORT && kind == GLOBAL_SCOPE)
    {
        printf("[Analyzer] [Debug] Statements can't be in a global scope...\n");
        exit(1);
    }
}

void handle_block(Node* node, SymbolTable* scope, int* local_offset)
{
    Node* next = node->block_node.block.statements->head;
    
    while (next != NULL)
    {
        if (next->type == NODE_DECLARATION || next->type == NODE_IF || next->type == NODE_WHILE_LOOP || next->type == NODE_FOR_LOOP)
        {
            analyze_node(next, scope, local_offset);
        }
        else
        {
            analyze_node(next, scope, NULL);
        }
        
        next = next->next;
    }
}

Type* get_function_return_type(SymbolTable* scope)
{
    SymbolTable* current = scope;

    while (current != NULL)
    {
        if (current->scope_kind == SYMBOL_FUNCTION && current->owner_statement != NULL)
        {
            return current->owner_statement->symbol_function->return_type;
        }

        current = current->parent;
    }

    return NULL;
}

void handle_return(Node* node, SymbolTable* scope)
{
    Node** return_value = &node->return_node.return_n.return_value;
    
    if (*return_value == NULL)
    {
        printf("[Analyzer] [Debug] Returning void...\n");
        return;
    }

    analyze_node(*return_value, scope, NULL);

    Type* return_type = get_function_return_type(scope);

    if (return_type == NULL)
    {
        printf("[Analyzer] [Debug] Function return type is null...\n");
        exit(1);
    }

    implictly_cast_all(return_type, *return_value, scope);
}

int get_list_size(Node* list_head)
{
    Node* next = list_head;

    int count = 0;

    while (next !=  NULL)
    {
        next = next->next;
        count++;
    }

    return count;
}

int get_type_chain_size(Type* head)
{
    Type* next = head;

    int count = 0;

    while (next !=  NULL)
    {
        next = next->base;
        count++;
    }

    return count;
}

int is_type_identic(Type* first, Type* second)
{
    if (get_type_chain_size(first) != get_type_chain_size(second))
    {
        return 0;
    }

    Type* type = first;
    Type* _type = second;

    while (type != NULL)
    {
        if (type->type != _type->type)
        {
            return 0;
        }

        type = type->base;
        _type = _type->base;
    }

    return 1;
}

void check_arguments(Symbol* symbol, Node* head, SymbolTable* scope)
{
    int arguments_size = head != NULL ? get_list_size(head) : 0;
    int params_size = get_list_size(symbol->symbol_function->params_head);
    
    if (arguments_size != params_size)
    {
        printf("[Analyzer] [Debug] Expected argument size is: %d...\n", params_size);
        exit(1);
    }

    Node* current = head;
    Node* current_ = symbol->symbol_function->params_head;

    while (current != NULL)
    {
        Type* type = type_of_expression(current->argument_node.argument.value, scope);
        Type* type_ = current_->param_node.param.argument_type;

        if (!is_type_identic(type, type_))
        {
            printf("[Analyzer] [Debug] Types aren't the same...\n");
            exit(1);
        }
        
        current = current->next;
        current_ = current_->next;
    }
}

/**
 * TODO: Terminar e implementar esse metodo (checar se está funcionando antes)
 *
 * - Ficar atento ao uso do check arguments.
 * - Ficar atento ao uso do callee implementado no node de function call.
 */
void handle_function_call(Node* node, SymbolTable* scope)
{
    
}

void handle_if(Node* node, SymbolTable* scope, int* offset)
{
    analyze_node(node->if_node.if_node.condition_top, scope, NULL);

    if (type_of_expression(node->if_node.if_node.condition_top, scope)->type != TYPE_BOOL)
    {
        printf("[Analyzer] [Debug] Invalid if condition, expression return type needs to be 'boolean'...\n");
        exit(1);
    }

    SymbolTable* then_scope = create_scope(SYMBOL_IF, scope, NULL);
    analyze_node(node->if_node.if_node.then_branch, then_scope, offset);

    if (node->if_node.if_node.else_branch != NULL)
    {
        SymbolTable* else_scope = create_scope(SYMBOL_ELSE, scope, NULL);
        analyze_node(node->if_node.if_node.else_branch, else_scope, offset);
    }
}

void handle_adress_of(Node* node, SymbolTable* scope)
{
    const AdressOfNode* adress_of = &node->adress_of_node.adress_of;
    
    if (adress_of->expression->type != NODE_IDENTIFIER)
    {
        printf("[Analyzer] [Debug] Adressing requires a pointer variable...\n");
        exit(1);
    }
    
    Symbol* var = find_symbol_from_scope(adress_of->expression->variable_node.variable.identifier, scope, 0, 0);
    
    if (var == NULL)
    {
        printf("[Analyzer] [Debug] Referencing a not declared variable...\n");
        exit(1);
    }
}

void handle_dereference(Node* node, SymbolTable* scope)
{
    Node* ref = node;

    while (ref->type == NODE_DEREFERENCE)
    {
        ref = ref->dereference_node.dereference.ptr;
    }

    if (ref->type != NODE_IDENTIFIER)
    {
        printf("[Analyzer] [Debug] Dereferencing requires a pointer variable...\n");
        exit(1);
    }
    
    Symbol* var = find_symbol_from_scope(ref->variable_node.variable.identifier, scope, 0, 0);
    
    if (var == NULL)
    {
        printf("[Analyzer] [Debug] Dereferencing a not declared variable...\n");
        exit(1);
    }
}

void handle_while_loop(Node* node, SymbolTable* scope, int* offset)
{
    analyze_node(node->while_node.while_loop.condition, scope, NULL);

    if (type_of_expression(node->while_node.while_loop.condition, scope)->type != TYPE_BOOL)
    {
        printf("[Analyzer] [Debug] Invalid while condition, expression return type needs to be 'boolean'...\n");
        exit(1);
    }

    SymbolTable* block_scope = create_scope(SYMBOL_WHILE, scope, NULL);
    analyze_node(node->while_node.while_loop.then_block, block_scope, offset);
}

void handle_for_loop(Node* node, SymbolTable* scope, int* offset)
{
    SymbolTable* block_scope = create_scope(SYMBOL_FOR, scope, NULL);
    
    analyze_node(node->for_node.for_loop_node.init, block_scope, offset);

    analyze_node(node->for_node.for_loop_node.condition, block_scope, offset);
    analyze_node(node->for_node.for_loop_node.then_statement, block_scope, offset);

    if (type_of_expression(node->for_node.for_loop_node.condition, block_scope)->type != TYPE_BOOL)
    {
        printf("[Analyzer] [Debug] Invalid for condition, expression return type needs to be 'boolean'...\n");
        exit(1);
    }

    analyze_node(node->for_node.for_loop_node.then_block, block_scope, offset);
}

int is_inside(SymbolTable* scope, SymbolType type)
{
    if (scope == NULL)
    {
        return 0;
    }

    if (scope->scope_kind == type)
    {
        return 1;
    }

    return is_inside(scope->parent, type);
}

void handle_break(Node* node, SymbolTable* scope)
{
    if (!is_inside(scope, SYMBOL_FOR) && !is_inside(scope, SYMBOL_WHILE))
    {
        printf("[Analyzer] [Debug] Break statement outside a loop statement...\n");
        exit(1);
    }
}

void handle_var_assign(Node* node, SymbolTable* scope)
{
    analyze_node(node->variable_assign_node.assin_node.left, scope, NULL); // analyze function already checks if things exist...
    analyze_node(node->variable_assign_node.assin_node.assign_value, scope, NULL);
}

static Node* get_function_from_class(Symbol* class, char* func_name)
{
    for (int i = 0; i < class->symbol_class->func_count; i++)
    {
        Node* member = class->symbol_class->functions[i];

        if (strcmp(member->function_node.function.identifier, func_name) == 0)
        {
            return member;
        }
    }

    return NULL;
}

int class_extends(Symbol* class, char* super_name)
{
    if (class->symbol_class->super == NULL)
    {
        return 0;
    }

    if (strcmp(class->symbol_class->super->symbol_class->identifier, super_name) == 0)
    {
        return 1;
    }

    return class_extends(class->symbol_class->super, super_name);
}

int is_assignable(Symbol* from, Symbol* to)
{
    if (strcmp(from->symbol_class->identifier, to->symbol_class->identifier) == 0)
    {
        return 1;
    }
    
    return class_extends(from, (char*) to->symbol_class->identifier);
}

static Node* get_member_from_class(Symbol* class, char* member_name)
{
    if (class == NULL)
    {
        return NULL;
    }
    
    for (int i = 0; i < class->symbol_class->field_count; i++)
    {
        Node* member = class->symbol_class->fields[i];

        if (strcmp(member->declare_node.declare.identifier, member_name) == 0)
        {
            return member;
        }
    }

    return get_member_from_class(class->symbol_class->super, member_name);
}

void handle_member_acess(Node* node, SymbolTable* scope)
{
    Type* type = type_of_expression(node, scope);

    if (type == NULL)
    {
        exit(1);
    }
}

void handle_class_funcs(Node* node, SymbolTable* scope)
{
    const ClassNode* class_node = &node->class_node.class_node;
    
    for (int i = 0; i < class_node->func_count; i++)
    {
        analyze_node(class_node->func_declare_list[i], scope, NULL);
    }
}

void handle_class_vars(Node* node, SymbolTable* scope)
{
    const ClassNode* class_node = &node->class_node.class_node;
    
    for (int i = 0; i < class_node->var_count; i++)
    {
        analyze_node(class_node->var_declare_list[i], scope, NULL);
    }
}

void handle_class_declaration(Node* node, SymbolTable* scope)
{
    const ClassNode* class_node = &node->class_node.class_node;

    if (find_symbol_from_scope(class_node->identifer, scope, 0, 1) != NULL)
    {
        printf("[Analyzer] [Debug] Class already declared: %s...", class_node->identifer);
        exit(1);
    }

    Symbol* class_symbol = create_symbol(SYMBOL_CLASS, node, scope, 0, NULL);

    SymbolTable* class_scope = create_scope(SYMBOL_CLASS, scope, class_symbol);

    if (class_node->super_identifer != NULL)
    {
        Symbol* super_symbol = find_symbol_from_scope(class_node->super_identifer, scope, 0, 1);

        if (super_symbol == NULL)
        {
            printf("[Analyzer] [Debug] Super class not found...\n");
            exit(1);
        }

        class_symbol->symbol_class->super = super_symbol;
    }
    else 
    {
        printf("[Analyzer] [Debug] Class dont have a super class...\n");
    }

    class_symbol->symbol_class->class_scope = class_scope;
    class_symbol->symbol_class->constructor = NULL;

    if (class_node->constructor != NULL)
    {
        analyze_node(class_node->constructor, class_scope, NULL);
        class_symbol->symbol_class->constructor = find_symbol_from_scope(class_node->constructor->function_node.function.identifier, class_scope, 1, 0);
    }

    handle_class_vars(node, class_scope);
    handle_class_funcs(node, class_scope);
}

void handle_create_instance(Node* node, SymbolTable* scope)
{
    const CreateInstanceNode* instance_node = &node->create_instance_node.create_instance;

    Symbol* class = find_symbol_from_scope(instance_node->class_name, scope, 0, 1);

    if (class == NULL)
    {
        printf("[Analyzer] [Debug] Failed to create a instance, class not found: %s...", instance_node->class_name);
        exit(1);
    }

    if (class->symbol_class->constructor != NULL)
    {
        check_arguments(class->symbol_class->constructor, instance_node->constructor_args == NULL ? NULL : instance_node->constructor_args->head, scope);
    }
}

void handle_this(Node* node, SymbolTable* scope)
{
    if (scope->scope_kind != SYMBOL_CLASS)
    {
        printf("[Analyzer] [Debug] This pointer needs to be used inside a class scope...\n");
        exit(1);
    }
}

/**
 * TODO: Terminar a implementação de classes.
 */
void analyze_node(Node* node, SymbolTable* scope, int* offset)
{
    check_global_scope(node, scope);
    
    switch (node->type) 
    {
        case NODE_DECLARATION:
        {
            handle_variable_declaration(node, scope, offset);

            return;
        }
        case NODE_FUNCTION:
        {
            handle_function_declaration(node, scope);

            return;
        }
        case NODE_LITERAL:
        {
            return;
        }
        case NODE_IDENTIFIER:
        {
            return;
        }
        case NODE_IF:
        {
            handle_if(node, scope, offset);

            return;
        }
        case NODE_FUNCTION_CALL:
        {
            handle_function_call(node, scope);
            
            return;
        }
        case NODE_WHILE_LOOP:
        {
            handle_while_loop(node, scope, offset);

            return;
        }
        case NODE_FOR_LOOP:
        {
            handle_for_loop(node, scope, offset);

            return;
        }
        case NODE_OPERATION:
        {
            handle_operation(node, scope);

            return;
        }
        case NODE_BLOCK:
        {
            handle_block(node, scope, offset);

            return;
        }
        case NODE_RETURN:
        {
            handle_return(node, scope);

            return;
        }
        case NODE_BREAK:
        {
            handle_break(node, scope);

            return;
        }
        case NODE_ADRESS_OF:
        {
            handle_adress_of(node, scope);

            return;
        }
        case NODE_DEREFERENCE:
        {
            handle_dereference(node, scope);

            return;
        }
        case NODE_VARIABLE_ASSIGN:
        {
            handle_var_assign(node, scope);

            return;
        }
        case NODE_MEMBER_ACCESS:
        {
            printf("[Analyzer] [Debug] Trying to acess a member from class object...\n");
            handle_member_acess(node, scope);

            return;
        }
        case NODE_CLASS:
        {
            handle_class_declaration(node, scope);

            return;
        }
        case NODE_CREATE_INSTANCE:
        {
            handle_create_instance(node, scope);

            return;
        }
        case NODE_THIS:
        {
            handle_this(node, scope);

            return;
        }
        default:
        {
            printf("[Analyzer] [Debug] Node type not implemented in analyzer: \"%d\"...", node->type);
            exit(1);
        }
    }
}
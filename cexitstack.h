#pragma once

#define CEXITSTACK_CONDITION_ALWAYS 0
#define CEXITSTACK_CONDITION_ERROR 1
#define CEXITSTACK_DEFAULT_INITIAL_CAPACITY 10
#define CEXITSTACK_DEFAULT_CAPACITY_INCREMENT 10

typedef void cexitstack_func( void * );

typedef struct _cexitstack_item {
    void *object;
    unsigned int condition;
    cexitstack_func *func;
} cexitstack_item;

typedef struct _cexitstack {
    unsigned int length;
    unsigned int capacity;
    unsigned int stack_allocated;
    cexitstack_item *items;
} cexitstack;

cexitstack *cexitstack_new( unsigned int initial_length );
int cexitstack_init( cexitstack *stack, unsigned int initial_length );
int cexitstack_return( cexitstack *stack, int return_val, unsigned int condition );
int cexitstack_push_full( cexitstack *stack, void *object, unsigned int condition, cexitstack_func *func );
int cexitstack_push_struct( cexitstack *stack, cexitstack_item *item );
void cexitstack_free( cexitstack *stack );
void cexitstack_func_free( void *object );

#define cexitstack_push(X, Y, ...) _Generic((Y), \
    cexitstack_item *: cexitstack_push_struct,   \
    void *: cexitstack_push_full,                \
    default: cexitstack_push_full                \
) ((X), (Y), ## __VA_ARGS__);

#define CEXITSTACK(name, n)      \
struct {                         \
    unsigned int length;         \
    unsigned int capacity;       \
    cexitstack_item items[(n)];  \
} (name) = { .capacity=(n) };

#define CEXITSTACK_PUSH(stack, obj, cond, fun) { \
if ((stack).capacity <= (stack).length) abort(); \
(stack).items[(stack).length++] = (cexitstack_item){ .object = (obj), .condition = (cond), .func = (fun) }; }

#define CEXITSTACK_RETURN(stack, cond, retval) {                                      \
int avoid_using_this_macro_internal_variable = (stack).length;                        \
while (avoid_using_this_macro_internal_variable-- > 0) {                              \
    cexitstack_item *item = (stack).items + avoid_using_this_macro_internal_variable; \
    if (item->condition == CEXITSTACK_CONDITION_ALWAYS || (cond) & item->condition)   \
        ( *item->func )( item->object );                                          \
}                                                                                     \
return (retval); }


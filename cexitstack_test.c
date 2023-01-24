#include <stdlib.h>
#include <assert.h>

#include "cexitstack.h"

void test_new_default( void )
{
    cexitstack *stack = cexitstack_new( 0 );
    assert( stack != NULL );
    assert( stack->items != NULL );
    assert( stack->capacity == CEXITSTACK_DEFAULT_INITIAL_CAPACITY );
    assert( stack->length == 0 );
    assert( stack->stack_allocated == 1 );
    for (unsigned int i = 0; i < stack->capacity; i++)
        assert( stack->items[i].object == 0 && stack->items[i].func == 0 && stack->items[i].condition == 0 );
    free( stack->items );
    free( stack );
}

void test_new( void )
{
    cexitstack *stack = cexitstack_new( 12 );
    assert( stack->items != NULL );
    assert( stack->capacity == 12 );
    assert( stack->length == 0 );
    assert( stack->stack_allocated == 1 );
    for (unsigned int i = 0; i < stack->capacity; i++)
        assert( stack->items[i].object == 0 && stack->items[i].func == 0 && stack->items[i].condition == 0 );
    free( stack->items );
    free( stack );
}

void test_init_default( void )
{
    cexitstack stack;
    assert( cexitstack_init( &stack, 0 ) != 0 );
    assert( stack.capacity == CEXITSTACK_DEFAULT_INITIAL_CAPACITY );
    assert( stack.items != NULL );
    assert( stack.length == 0 );
    assert( stack.stack_allocated == 0 );
    for (unsigned int i = 0; i < stack.capacity; i++)
        assert( stack.items[i].object == 0 && stack.items[i].func == 0 && stack.items[i].condition == 0 );
    free( stack.items );
}

void test_init( void )
{
    cexitstack stack;
    assert( cexitstack_init( &stack, 12 ) != 0 );
    assert( stack.capacity == 12 );
    assert( stack.items != NULL );
    assert( stack.length == 0 );
    assert( stack.stack_allocated == 0 );
    for (unsigned int i = 0; i < stack.capacity; i++)
        assert( stack.items[i].object == 0 && stack.items[i].func == 0 && stack.items[i].condition == 0 );
    free( stack.items );
}

void test_push_one_struct( void )
{
    cexitstack stack;
    assert( cexitstack_init( &stack, 0 ) != 0 );
    int i = 2;
    cexitstack_item item = {
        .condition = 1,
        .func = &cexitstack_func_free,
        .object = &i
    };
    cexitstack_push_struct( &stack, &item );
    assert( stack.length == 1 );
    assert( stack.items[0].condition == 1 && stack.items[0].func == &cexitstack_func_free && stack.items[0].object == &i );
    assert( *(int *)(stack.items[0].object) == i );
    free( stack.items );
}

void test_push_one_full( void )
{
    cexitstack stack;
    assert( cexitstack_init( &stack, 0 ) != 0 );
    int i = 2;
    cexitstack_push_full( &stack, &i, 1, &cexitstack_func_free );
    assert( stack.length == 1 );
    assert( stack.items[0].condition == 1 && stack.items[0].func == &cexitstack_func_free && stack.items[0].object == &i );
    assert( *(int *)(stack.items[0].object) == i );
    free( stack.items );
}

#define TEST_PUSH_MULTIPLE_N 5
void test_push_multiple_struct( void )
{
    cexitstack stack;
    assert( cexitstack_init( &stack, 0 ) != 0 );
    int objects[TEST_PUSH_MULTIPLE_N] = { 10, 11, 12, 13, 14 };
    for (int i = 0; i < TEST_PUSH_MULTIPLE_N; i++) {
        cexitstack_item item = { .condition = i, .func = &cexitstack_func_free, .object = objects + i };
        cexitstack_push_struct( &stack, &item );
    }
    assert( stack.length == TEST_PUSH_MULTIPLE_N );
    for (int i = 0; i < TEST_PUSH_MULTIPLE_N; i++)
        assert( stack.items[i].condition == i && stack.items[i].func == &cexitstack_func_free && *(int *)stack.items[i].object == objects[i] );
    free( stack.items );
}

void test_push_multiple_full( void )
{
    cexitstack stack;
    assert( cexitstack_init( &stack, 0 ) != 0 );
    int objects[TEST_PUSH_MULTIPLE_N] = { 10, 11, 12, 13, 14 };
    for (int i = 0; i < TEST_PUSH_MULTIPLE_N; i++)
        cexitstack_push_full( &stack, objects + i, i, &cexitstack_func_free );
    assert( stack.length == TEST_PUSH_MULTIPLE_N );
    for (int i = 0; i < TEST_PUSH_MULTIPLE_N; i++)
        assert( stack.items[i].condition == i && stack.items[i].func == &cexitstack_func_free && *(int *)stack.items[i].object == objects[i] );
    free( stack.items );
}

#define TEST_REALLOC_N 2
void test_realloc( void )
{
    cexitstack stack;
    assert( cexitstack_init( &stack, TEST_REALLOC_N ) != 0 );
    int objects[TEST_PUSH_MULTIPLE_N] = { 10, 11, 12, 13, 14 };
    unsigned int old_capacity = stack.capacity;
    for (int i = 0; i < TEST_PUSH_MULTIPLE_N; i++)
        cexitstack_push_full( &stack, objects + i, i, &cexitstack_func_free );
    assert( stack.capacity > old_capacity );
    assert( stack.length == TEST_PUSH_MULTIPLE_N );
    for (int i = 0; i < TEST_PUSH_MULTIPLE_N; i++)
        assert( stack.items[i].condition == i && stack.items[i].func == &cexitstack_func_free && *(int *)stack.items[i].object == objects[i] );
    free( stack.items );
}

void test_faulty_input( void )
{
    cexitstack stack = { .capacity = 0, .items = NULL };
    assert( cexitstack_init( NULL, 0 ) == 0 );
    assert( cexitstack_push_full( &stack, 0, 0, 0 ) == 0 );
    assert( cexitstack_push_struct( &stack, NULL ) == 0 );
}

void test_cexitstack_func_free( void )
{
    cexitstack_func *fun = &cexitstack_func_free;
    int *i = calloc( 1, sizeof( int ) );
    (*fun)(i);
}

void cexitstack_func_set( void *target )
{
    *(int *)target = 1;
}

#define TEST_RETURN_SET_N 5
void test_return_one_condition( void )
{
    cexitstack stack;
    assert( cexitstack_init( &stack, 0 ) != 0 );
    int results[TEST_RETURN_SET_N] = { 0 };
    for (int i = 0; i < TEST_RETURN_SET_N; i++)
        cexitstack_push_full( &stack, results + i, 1, &cexitstack_func_set );
    assert( cexitstack_return( &stack, -1, 1 ) == -1 );
    for (int i = 0; i < TEST_RETURN_SET_N; i++)
        assert( results[i] == 1 );
}

void test_return_multiple_conditions_all( void )
{
    cexitstack stack;
    assert( cexitstack_init( &stack, 0 ) != 0 );
    int results[TEST_RETURN_SET_N] = { 0 };
    int conditions[TEST_RETURN_SET_N] = { 2, 3, 6, 2, CEXITSTACK_CONDITION_ALWAYS };
    int expect[TEST_RETURN_SET_N] = { 1, 1, 1, 1, 1 };
    for (int i = 0; i < TEST_RETURN_SET_N; i++)
        cexitstack_push_full( &stack, results + i, conditions[i], &cexitstack_func_set );
    assert( cexitstack_return( &stack, -1, 2 ) == -1 );
    for (int i = 0; i < TEST_RETURN_SET_N; i++)
        assert( results[i] == expect[i] );
}

void test_return_multiple_conditions_partial( void )
{
    cexitstack stack;
    assert( cexitstack_init( &stack, 0 ) != 0 );
    int results[TEST_RETURN_SET_N] = { 0 };
    int conditions[TEST_RETURN_SET_N] = { 3, 1, CEXITSTACK_CONDITION_ALWAYS, 2, 4 };
    int expect[TEST_RETURN_SET_N] = { 1, 0, 1, 1, 0 };
    for (int i = 0; i < TEST_RETURN_SET_N; i++)
        cexitstack_push_full( &stack, results + i, conditions[i], &cexitstack_func_set );
    assert( cexitstack_return( &stack, -1, 2 ) == -1 );
    for (int i = 0; i < TEST_RETURN_SET_N; i++)
        assert( results[i] == expect[i] );
}

void test_return_free_empty( void )
{
    cexitstack *stack = cexitstack_new( 0 );
    assert( stack && stack->stack_allocated );
    assert( cexitstack_return( stack, -1, 0 ) == -1 );
}

void test_generic_push_macro( void )
{
    cexitstack stack;
    assert( cexitstack_init( &stack, 0 ) != 0 );
    cexitstack_item item0 = { .condition = 1, .func = &cexitstack_func_free, .object = (void *)2 };
    cexitstack_item item1 = { .condition = 3, .func = &cexitstack_func_free, .object = (void *)4 };
    cexitstack_push( &stack, &item0 );
    cexitstack_push( &stack, item1.object, item1.condition, item1.func );
    assert( stack.length == 2 );
    assert( stack.items[0].object == item0.object && stack.items[0].func == item0.func && stack.items[0].condition == item0.condition );
    assert( stack.items[1].object == item1.object && stack.items[1].func == item1.func && stack.items[1].condition == item1.condition );
    free( stack.items );
}

void test_macro_init( void )
{
    CEXITSTACK( stack, 10 );
    assert( stack.items != NULL );
    assert( stack.capacity == 10 );
    assert( stack.length == 0 );
    for (unsigned int i = 0; i < stack.capacity; i++)
        assert( stack.items[i].object == 0 && stack.items[i].func == 0 && stack.items[i].condition == 0 );
}

void test_macro_push( void )
{
    CEXITSTACK( stack, 5 );
    int objects[2] = { 1, 2 };
    CEXITSTACK_PUSH( stack, &objects[0], 2, &cexitstack_func_free );
    CEXITSTACK_PUSH( stack, &objects[1], 3, &cexitstack_func_free );
    assert( stack.capacity == 5 && stack.length == 2 );
    assert( stack.items[0].condition == 2 && *(int *)(stack.items[0].object) == 1 && stack.items[0].func == &cexitstack_func_free );
    assert( stack.items[1].condition == 3 && *(int *)(stack.items[1].object) == 2 && stack.items[1].func == &cexitstack_func_free );
}

#define TEST_MACRO_RETURN_N 5
int test_macro_return_internal( int *results )
{
    CEXITSTACK( stack, TEST_MACRO_RETURN_N );
    int conditions[TEST_MACRO_RETURN_N] = { 3, 1, CEXITSTACK_CONDITION_ALWAYS, 2, 4 };
    for (int i = 0; i < TEST_MACRO_RETURN_N; i++)
        CEXITSTACK_PUSH( stack, results + i, conditions[i], &cexitstack_func_set );
    assert( stack.length == TEST_MACRO_RETURN_N );
    CEXITSTACK_RETURN( stack, -1, 2 );
}

void test_macro_return( void )
{
    int results[TEST_MACRO_RETURN_N] = { 0 };
    int expect[TEST_MACRO_RETURN_N] = { 1, 0, 1, 1, 0 };
    assert( test_macro_return_internal( results ) == -1 );
    for (int i = 0; i < TEST_MACRO_RETURN_N; i++)
        assert( results[i] == expect[i] );
}

int main( int argc, char **argv )
{
    test_new_default();
    test_new();
    test_init_default();
    test_init();
    test_push_one_struct();
    test_push_one_full();
    test_push_multiple_struct();
    test_push_multiple_full();
    test_realloc();
    test_faulty_input();
    test_cexitstack_func_free();
    test_return_one_condition();
    test_return_multiple_conditions_all();
    test_return_multiple_conditions_partial();
    test_return_free_empty();
    test_generic_push_macro();
    test_macro_init();
    test_macro_push();
    test_macro_return();
}

#include <stdlib.h>
#include <string.h>

#include "cexitstack.h"

int cexitstack_expand( cexitstack *stack, unsigned int added_capacity );

extern cexitstack *
cexitstack_new( unsigned int initial_length )
{
    cexitstack *stack = calloc( 1, sizeof( cexitstack ) );
    if (!stack) return NULL;
    if (!cexitstack_init( stack, initial_length )) {
        free( stack );
        return NULL;
    }
    stack->stack_allocated = 1;
    return stack;
}

extern int
cexitstack_init( cexitstack *stack, unsigned int initial_length )
{
    if (!stack) return 0;
    memset( stack, 0, sizeof( cexitstack ) );
    stack->capacity = initial_length ? initial_length : CEXITSTACK_DEFAULT_INITIAL_CAPACITY;
    stack->items = calloc( stack->capacity, sizeof( cexitstack_item ) );
    if (!stack->items) return 0;
    return 1;
}

extern int
cexitstack_return( cexitstack *stack, int return_val, unsigned int condition )
{
    if (stack->items && stack->length > 0) {
        int i = stack->length;
        while (i-- > 0) {
            cexitstack_item *item = stack->items + i;
            if (item->condition == CEXITSTACK_CONDITION_ALWAYS || condition & item->condition)
                ( *item->func )( item->object );
        }
    }
    cexitstack_free( stack );
    return return_val;
}

extern int
cexitstack_push_full( cexitstack *stack, void *object, unsigned int condition, cexitstack_func *func )
{
    if (!stack->items) return 0;
    if (stack->length == stack->capacity)
        if (!cexitstack_expand( stack, 0 ))
            return 0;
    cexitstack_item *new_item = &stack->items[stack->length];
    stack->items[stack->length] = ( cexitstack_item ){ .object = object, .condition = condition, .func = func };
    stack->length++;
    return 1;
}

extern int
cexitstack_push_struct( cexitstack *stack, cexitstack_item *item )
{
    if (!stack->items || !item) return 0;
    if (stack->length == stack->capacity)
        if (!cexitstack_expand( stack, 0 ))
            return 0;
    stack->items[stack->length] = *item;
    stack->length++;
    return 1;
}

extern void
cexitstack_func_free( void *object )
{
    free( object );
}

int
cexitstack_expand( cexitstack *stack, unsigned int added_capacity )
{
    if (!stack->items || !stack->capacity) return 0;
    unsigned int new_capacity = stack->capacity + ( added_capacity ? added_capacity : CEXITSTACK_DEFAULT_CAPACITY_INCREMENT );
    cexitstack_item *new_items = realloc( stack->items, sizeof( cexitstack_item ) * new_capacity );
    if (!new_items) return 0;
    stack->items = new_items;
    stack->capacity = new_capacity;
    return 1;
}

extern void
cexitstack_free( cexitstack *stack )
{
    if (stack) {
        if (stack->items)
            free( stack->items );
        if (stack->stack_allocated)
            free( stack );
    }
}
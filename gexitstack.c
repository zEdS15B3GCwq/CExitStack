#include <glib.h>

#include "gexitstack.h"

static void gexitstack_item_destroy( gexitstack_item *const item );

inline gexitstack *
gexitstack_new( void )
{
    GArray *stack = g_array_sized_new( FALSE, TRUE, sizeof( gexitstack_item ), GEXITSTACK_DEFAULT_INITIAL_CAPACITY );
    //g_array_set_clear_func( stack, NULL );
    return stack;
}

inline int
gexitstack_return( gexitstack *stack, const gint return_val, const guint condition )
{
    for (guint i = stack->len; i > 0; i--) {
        gexitstack_item *item = &g_array_index( stack, gexitstack_item, i - 1 );
        if (item->condition == GEXITSTACK_CONDITION_ALWAYS || condition & item->condition)
            gexitstack_item_destroy( item );
    }
    g_array_unref( stack );
    return return_val;
}

inline gexitstack *
gexitstack_push_full( gexitstack *stack, const gpointer object, guint condition, const GDestroyNotify func )
{
    const gexitstack_item new_item = ( gexitstack_item ){ .object = object, .condition = condition, .func = func };
    return g_array_append_vals( stack, &new_item, 1 );
}

inline gexitstack *
gexitstack_push_struct( gexitstack *stack, const gexitstack_item *item )
{
    return g_array_append_vals( stack, item, 1 );
}

inline void
gexitstack_free( gexitstack **stack )
{
    g_clear_pointer( stack, g_array_unref );
}

static void
gexitstack_item_destroy( gexitstack_item *const item )
{
    g_clear_pointer( &item->object, item->func );
    item->condition = 0;
    item->func = NULL;
}
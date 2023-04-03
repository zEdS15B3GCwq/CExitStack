#pragma once
#ifndef GEXITSTACK_H
#define GEXITSTACK_H

#include <glib.h>

#define GEXITSTACK_CONDITION_ALWAYS 0
#define GEXITSTACK_CONDITION_ERROR 1
#define GEXITSTACK_DEFAULT_INITIAL_CAPACITY 10

typedef struct _gexitstack_item
{
    gpointer object;
    guint condition;
    GDestroyNotify func;
} gexitstack_item;

typedef GArray gexitstack;

inline gexitstack *gexitstack_new( void );
inline int gexitstack_return( gexitstack *stack, gint return_val, guint condition );
inline gexitstack *gexitstack_push_full( gexitstack *stack, gpointer object, guint condition, GDestroyNotify func );
inline gexitstack *gexitstack_push_struct( gexitstack *stack, gexitstack_item *item );
inline void gexitstack_free( gexitstack *stack );

#define gexitstack_push(X, Y, ...) _Generic((Y), \
    gexitstack_item *: gexitstack_push_struct,   \
    gpointer: gexitstack_push_full               \
) ((X), (Y), ## __VA_ARGS__);

#endif
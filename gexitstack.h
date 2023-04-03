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

extern inline gexitstack *gexitstack_new( void );
extern inline int gexitstack_return( gexitstack *stack, const gint return_val, const guint condition );
extern inline gexitstack *gexitstack_push_full( gexitstack *stack, const gpointer object, guint condition, const GDestroyNotify func );
extern inline gexitstack *gexitstack_push_struct( gexitstack *stack, const gexitstack_item *item );
extern inline void gexitstack_free( gexitstack **stack );

#define gexitstack_push(X, Y, ...) _Generic((Y), \
    gexitstack_item *: gexitstack_push_struct,   \
    default: gexitstack_push_full                \
) ((X), (Y), ## __VA_ARGS__);

#endif
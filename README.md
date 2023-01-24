# CExitStack
Inspired by ExitStack in Python and *defer* in other languages.

It's in working state but I haven't had enough experience with it to be able to say if it's worth using.

NB This doc has not been checked for errors; included code segments are sloppy and not intended to be working examples.

## What's this?

Allocated resources need to be freed when not in use anymore, e.g. allocated memory is freed to avoid memory leaks, files need to be closed, etc. Some languages provide easy ways to handle this, like Python's `with` contexts or `defer` in others (e.g. Golang), to name a few. Sometimes, the need to free the resources is conditional, e.g. some resource is allocated and initialised, and it might only be freed in the same code block if there's an error, otherwise it's used for a while and freed later. In C, the gist of handling such cases in a simple way could look like this:

```C
result = allocate(intermediate_resource);
if (result == ERROR) goto error1;
result = allocate(final_resource);
if (result == ERROR) goto error2;
free(intermediate_resource);
return final_resource;
:error2
free(final_resource);
:error1
free(intermediate_resource);
return ERROR
```

The advantage of using `goto` is its simplicity, and the code can be easy to read as long as the error handling isn't very complicated. However, if there are more resources or more processing steps involved, with possible error branches, things can get messy.

`CExitStack` was inspired by Python's `contextlib.ExitStack` and `defer`, in which it provides a *stack* where one can push several function calls, and which can be *popped* manually or when returning from a function. One difference is, however, that `CExitStack` also stores a *condition* field, that allows selective execution. It's nothing fancy, just a bitwise AND operation between the stored condition and the provided one, but this way is a bit more flexible than `ExitStack` or `defer`, e.g. some resources can be freed on error, some always and so on.

## How to use

- You need `cexitstack.h` and `cexitstack.c` from this project - actually, if you only use the MACROs (see below), the header file is enough.

### Use case A, dynamically allocating the stack

```C
int yourfunction() {
    cexitstack *stack = cexitstack_new( 0 ); // 0 means default initial size
    yourtype *A = calloc( 1, sizeof(yourtype) );
    cexitstack_push( stack, (void *)A, CEXITSTACK_CONDITION_ALWAYS, &cexitstack_func_free ); // similar to "ExitStack.push(free(A))"
    // do things with A
    return cexitstack_return( stack, YOUR_RETURN_CODE, CEXITSTACK_CONDITION_ALWAYS );
}
```

Here, `A` is a variable allocated with `calloc`, that later is freed by `free(A)`. A stack is created, `A` is pushed onto it specifying that `free()` will be called in all cases when it's popped. `cexitstack_func_free` is a wrapper around `free()` that accepts a `void *` pointer and calls `free()` with it. `CExitStack` is agnostic of item types and what the pushed function does - any `void (void *)` function will do. `CEXITSTACK_CONDITION_ALWAYS` == 0, and is a treated as a special case that matches any condition.

`cexitstack_new( 0 )` allocates the a default sized (n=10) array for stack items. This array is reallocated when the capacity is reached.

`cexitstack_return` pops all items and executes their functions if the provided condition matches; it always returns whatever return code is provided, the reason for this is so that it can be used after `return` instead of using up another code line. So, this does the same:

```C
cexitstack_return( stack, does_not_matter, CEXITSTACK_CONDITION_ALWAYS );
return YOUR_RETURN_CODE;
```

`cexitstack_push` is a generic macro that can handle two cases:
```C
cexitstack_push( stack, (void *)item, unsigned_int_condition, &funcpointer );
// or
cexitstack_item item = {.object = (void *)item, .condition = unsigned_int_condition, .func = &funcpointer };
cexitstack_push( stack, &item );
```

### Use case B, only stack items are allocated dynamically

```C
int yourfunction() {
    cexitstack stack;
    cexitstack_init( &stack, 0 );
    typeA *A = calloc( 1, sizeof(typeA) );
    // from here on it's the same
    cexitstack_push( stack, (void *)A, CEXITSTACK_CONDITION_ALWAYS, &cexitstack_func_free );
    // do things with A
    return cexitstack_return( stack, YOUR_RETURN_CODE, CEXITSTACK_CONDITION_ALWAYS );
}
```

The only difference between this and case A is that the stack struct itself isn't allocated dynamically. The stack item array still is allocated with `calloc`.

### Use case C, fixed-size array with MARCOs, no dynamic allocation

```C
int yourfunction() {
    CEXITSTACK( stack, 5 ); // need to specify exact capacity; will NOT grow automatically
    typeA *A = calloc( 1, sizeof(typeA) );
    CEXITSTACK_PUSH( stack, (void *)A, CEXITSTACK_CONDITION_ALWAYS, &cexitstack_func_free );
    // do stuff to A
    CEXITSTACK_RETURN( stack, YOUR_RETURN_CODE, CEXITSTACK_CONDITION_ALWAYS );
}
```

The macro `CEXITSTACK()` defines a local `CExitStack` variable named `stack` with a fixed item capacity of 5; `CEXITSTACK_PUSH` is pretty much the same as in the other cases; `CEXITSTACK_RETURN` is also similar but it includes the `return` instruction. Don't mix macro and function cases, the allocated stack types are not the same. The macros are intended to provide the simplest approach and thus avoid such frivolities as error checking or expandable stack capacity.

The earlier example with an intermediate resource and conditional freeing of the final resource could be written as below (warning, intentionally sloppy code). This time, the deallocating function is custom.

```C
void deallocate_resource( void *resource ) {
    do_my_own_deallocation( (restype *)resource );
    maybe_do_some_logging();
}

int yourfunction( restype *final_resource ) {
    CEXITSTACK( stack, 2 );
    intermediate_type *intermediate_resource = calloc( 1, sizeof(intermediate_type) );
    CEXITSTACK_PUSH( stack, (void *)intermediate_resource, 0, &cexitstack_func_free ); // condition 0 == always
    // do intermediate things
    allocate_resource( final_resource );
    CEXITSTACK_PUSH( stack, (void *)final_resource, 1, &deallocate_resource ); // condition 1 == specific error condition
    // do things
    if ERROR
        CEXITSTACK_RETURN( stack, YOUR_ERROR_CODE, 1 ); // deallocate final_resource and intermediate_resource
    else
        CEXITSTACK_RETURN( stack, YOUR_SUCCESS_CODE, 0 ); // only intermediate_resource is freed
}
```

`CEXITSTACK_RETURN` can use any return type (`cexitstack_return` is restricted to `int`):

```C
restype *yourfunction() {
    ...
    CEXITSTACK_RETURN( stack, &final_resource, 0 );
}
```

## Is this any good?

No idea, I just thought it might be nice to be able to avoid `goto` and found the idea of `contextlib.ExitStack` and `defer` cool, so I threw this together. I'll still need to use it in some projects to see if I'll find this way more convenient.
    

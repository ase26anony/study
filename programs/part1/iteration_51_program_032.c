/* test_gty.h - Primary header file for gengtype parser coverage */

#ifndef TEST_GTY_H
#define TEST_GTY_H

#include "test_nested.h"
#include "test_macros.h"

/* Complex typedef with deeply nested parentheses for function pointers */
typedef int (*complex_func_ptr_t)(int (*inner)(char[10]), 
                                  struct {int a; int b;});

/* Multi-dimensional array with parenthesized size expression */
typedef int matrix_t[5][(sizeof(long) > 4) ? 10 : 20];

/* Struct within typedef containing arrays and nested struct */
typedef struct GTY((user)) {
    int id;
    char name[(10 + 2)];
    struct {
        int x;
        int y;
        int z[3];
    } coordinates;
    union {
        float f;
        double d;
        char bytes[sizeof(double)];
    } data;
} complex_struct_t;

/* Nested union with struct containing arrays */
typedef union GTY((chain_next = "next", chain_prev = "prev")) {
    struct {
        int x;
        char arr[5][5];
    } s;
    long l;
    double matrix[2][(8 / 2)];
} nested_union_t;

/* Function pointer with attribute containing balanced parentheses */
typedef void (*__attribute__((stdcall, aligned(8))) api_function_t)(
    int param1,
    char param2[(sizeof(int) * 2)],
    struct { int a; double b; } param3
);

/* Pointer to array of function pointers */
typedef int (*(*array_of_func_ptrs_t[5]))(void);

/* Deeply nested with multiple balanced groups */
typedef struct GTY((skip)) {
    int (*compare)(const void *a, const void *b);
    void *items[10];
    struct {
        int count;
        int capacity;
        int (*allocator)(size_t size, 
                         __attribute__((aligned(16))) void *context);
    } metadata;
} container_t;

/* Using macros that expand to balanced token groups */
typedef GTY_USER_ARGS my_struct {
    int i;
    PTR_TO(NESTED_ARRAY) data_ptr;
    COMPLEX_PTR nested;
} my_struct_t;

/* Array of structs with bit-fields and arrays */
typedef struct GTY((for_user)) {
    unsigned int flags:8;
    unsigned int type:4;
    char name[20];
    int values[(10 * sizeof(int))];
    struct {
        short x;
        short y;
    } position;
} widget_t;

/* Function pointer returning pointer to array */
typedef int (*(*func_returning_array_ptr_t)(int size))[10];

/* Nested attributes with balanced parentheses */
typedef int __attribute__((aligned(16), 
                          packed, 
                          deprecated("Use new_type instead"))) 
    aligned_deprecated_int;

/* Complex pointer type with multiple indirections */
typedef struct node ***node_ptr_ptr_ptr_t;

#endif /* TEST_GTY_H */

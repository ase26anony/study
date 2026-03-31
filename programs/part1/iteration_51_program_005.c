/* test_gty.h - Primary header for gengtype parser coverage testing */

#ifndef TEST_GTY_H
#define TEST_GTY_H

/* Include secondary headers to test multi-file parsing */
#include "test_nested.h"
#include "test_macros.h"

/* ==================== */
/* Complex Type Definitions with Nested Balanced Tokens */
/* ==================== */

/* 1. Function pointer with deeply nested parameter lists */
typedef int (*complex_func_ptr)(
    int (*inner_callback)(char param[10], 
                         struct {int tag; union {int i; float f;};}),
    void (*another)(int matrix[5][(sizeof(long) > 4) ? 10 : 20]),
    __attribute__((aligned(16))) int
);

/* 2. GTY annotation with nested balanced tokens in arguments */
typedef GTY((user, 
            ptr_alias("node_ptr"), 
            desc("tag"), 
            param_is(struct node_tag))) 
struct node * GTY((skip, 
                   reorder("data", "next"), 
                   param_is(int[5][(10+2)]))) 
node_ptr_t;

/* 3. Struct with nested arrays and bit-fields */
GTY((chain_next = "next", 
     chain_prev = "prev", 
     deletable)) 
struct complex_list {
    struct complex_list * GTY((tag("0"))) next;
    struct complex_list * GTY((tag("1"))) prev;
    int data[3][(sizeof(void*) == 8) ? 16 : 8];
    unsigned int flags:4;
    unsigned int:4; /* Padding bit-field */
    char name[20];
    struct {
        int x;
        int y[(5+3)];
    } GTY((skip)) position;
};

/* 4. Union containing struct with array */
typedef union GTY((desc("%0.tag"))) {
    struct GTY((tag("0"))) {
        int length;
        char buffer[256];
    } string_data;
    struct GTY((tag("1"))) {
        int matrix[2][2][2];
        struct { int a; int b; } pair;
    } numeric_data;
    long raw;
} variant_t;

/* 5. Multi-dimensional array with parenthesized size expressions */
typedef int matrix_type_t[
    5][
    (sizeof(long) == 8) ? 10 : 5
    ][
    (__alignof__(double) > 4) ? 8 : 4
];

/* 6. Function pointer returning pointer to array */
typedef int (*(*callback_factory)(int mode))[
    (mode == 0) ? 10 : 20
];

/* 7. Nested typedef with attributes */
typedef int __attribute__((aligned(16), 
                          packed, 
                          vector_size(16))) 
aligned_vector_t[4];

/* 8. Complex function type with GCC attributes */
typedef void (__attribute__((stdcall, 
                            noreturn, 
                            format(printf, 1, 2))) 
            *api_function)(const char *fmt, ...);

/* 9. Struct with anonymous union/struct */
typedef struct GTY((user)) {
    union {
        struct {
            int x;
            int y;
        };
        long coordinates;
    };
    char name[(MAX_NAME_LEN + 1)];
} point_t;

/* 10. Macro-expanded complex type */
typedef PTR_TO(NESTED_ARRAY_TYPE) complex_array_ptr;

/* 11. GTY with macro arguments */
typedef struct GTY_USER_SPEC my_type {
    int id;
    char data[GTY_ARRAY_SIZE];
} my_type_t;

/* 12. Pointer to function returning pointer to function */
typedef int (*(*(*triple_indirect)(void))[5])(int, int);

/* 13. Array of function pointers */
typedef void (*handlers[
    MAX_HANDLERS
])(int event, void *data);

/* 14. Struct with flexible array member in nested struct */
typedef struct container {
    int count;
    struct {
        int id;
        char name[];
    } items[];
} container_t;

#endif /* TEST_GTY_H */

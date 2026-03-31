/* Primary test header for gengtype parser coverage */
#ifndef TEST_GTY_H
#define TEST_GTY_H

/* Include secondary headers to test multi-file parsing */
#include "test_nested.h"
#include "test_macros.h"

/* ==================== Complex Nested Type Definitions ==================== */

/* 1. Function pointer with deeply nested parameter lists */
typedef int (*complex_func_ptr)(
    int (*inner_callback)(char[10], void*),
    struct { 
        int a; 
        int b[(sizeof(long) > 4) ? 8 : 4];
    } param
);

/* 2. Multi-dimensional array with parenthesized size expressions */
typedef int matrix_type[5][(sizeof(long) > 4) ? 10 : 20][(1 << 3)];

/* 3. Struct within union within typedef with arrays and bit-fields */
typedef union {
    struct {
        int x:4;
        int y[(2+3)];
        char arr[5][5];
    } inner;
    long l;
    struct {
        double d;
        int matrix[2][(3*2)];
    } *ptr;
} nested_union_t;

/* 4. GTY annotation with nested balanced tokens */
typedef GTY((user)) struct node * GTY((skip)) node_ptr;

/* 5. GTY chain with array containing expression */
GTY((chain_next = "next", chain_prev = "prev")) 
struct linked_list {
    struct linked_list *next;
    struct linked_list *prev;
    int data[(10+2)];
    char buffer[sizeof(struct { int a; double b; })];
};

/* 6. Nested function pointer type with GTY */
typedef GTY(()) void (*signal_handler)(
    int sig,
    void (*cleanup)(char *buffer[256]),
    struct {
        GTY((skip)) void *context;
        int flags;
    } info
);

/* 7. Complex array of function pointers */
typedef int (*array_of_funcs[5])(
    char param[((2*3)+1)],
    struct { int x; } *
);

/* ==================== GCC Attributes with Balanced Parentheses ==================== */

/* 8. Typedef with GCC attributes containing nested parentheses */
typedef int __attribute__((aligned(16), packed, 
    deprecated("Use 'new_int_t' instead"))) aligned_int;

/* 9. Function pointer with stdcall attribute */
typedef void (*__attribute__((stdcall, 
    format(printf, 1, 2)))) api_function)(const char *, ...);

/* 10. Struct with alignment attribute containing expression */
struct GTY(()) __attribute__((aligned((sizeof(void*) * 2)))) aligned_struct {
    int data[((1 << 3) - 1)];
    char pad;
};

/* ==================== Include-Dependent Definitions ==================== */

/* Use types from included headers */
typedef PTR_TO(NESTED_ARRAY) complex_array_ptr;
typedef GTY_USER_ARGS my_struct *user_struct_ptr;

/* Complex type using macro expansions */
typedef GTY((desc("tag_%0"))) MACRO_WITH_PARENS struct tree_node *tree_ptr;

/* ==================== Edge Cases ==================== */

/* 11. Multiple levels of nesting with all delimiter types */
typedef struct {
    int (*compare)(const void *, const void *);
    union {
        int i;
        char s[((10) + (5))];
        struct {
            long *array[3][2];
        } nested;
    } data;
    void (*handlers[2])(
        int,
        char[][(sizeof(int) + 2)]
    );
} callback_container_t;

/* 12. GTY with nested parentheses in arguments */
typedef GTY((user, 
    ptr_alias("my_alias"), 
    desc("%0"))) struct complex_type * GTY((skip)) complex_ptr;

/* 13. Array type with computed size using sizeof nested struct */
typedef char buffer_type[sizeof(struct {
    int header;
    double payload[4];
    char trailer[8];
})];

/* 14. Function returning pointer to array */
typedef int (*func_returning_array_ptr(void))[10];

/* 15. Const-volatile qualified function pointer with attributes */
typedef int (__attribute__((const)) * const cv_fp)(
    volatile int *,
    const char restrict *[5]
);

#endif /* TEST_GTY_H */

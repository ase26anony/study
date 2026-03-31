/* Primary test header for gengtype parser coverage */
#ifndef TEST_GTY_H
#define TEST_GTY_H

#include "test_nested.h"
#include "test_macros.h"

/* Test 1: Complex function pointer with nested parameter lists */
typedef int (*complex_fp1)(int (*inner)(char[10]), 
                           struct {int a; int b;});

/* GTY annotation with nested tokens */
typedef GTY((user)) struct node * GTY((skip)) node_ptr;

/* Test 2: Multi-dimensional array with parenthesized size expressions */
typedef int matrix_t[5][(sizeof(long) > 4) ? 10 : 20];

/* Test 3: Struct within union within typedef */
typedef union {
    struct {
        int x;
        char arr[5];
    };
    long l;
    double d[(2+3)];
} nested_union_t GTY((tag("UNION")));

/* Test 4: Chain structure with array in GTY */
GTY((chain_next = "next", chain_prev = "prev"))
struct list {
    struct list *next;
    struct list *prev;
    int data[(10+2)];
    char buffer[sizeof(struct {int a; double b;}) > 16 ? 20 : 10];
};

/* Test 5: Nested function pointer type */
typedef void (*(*nested_func_ptr)(int, char*))(double, 
    struct {int x; int y;});

/* Test 6: GCC attributes with balanced parentheses */
typedef int __attribute__((aligned(16), packed)) aligned_int;
typedef void (*__attribute__((stdcall)) api_fn)(int, 
    __attribute__((nonnull)) char**);

/* Test 7: Multiple levels of nesting */
typedef struct {
    union {
        int (*fp)(int[3], char (*)(void));
        struct {
            long array[5][(sizeof(int)*2)];
            short s;
        };
    } u;
    int (*callback)(struct {int a; int b;}[2]);
} mega_nested_t GTY((desc("%0.u.fp ? 1 : 0")));

/* Include secondary complex type */
#include "test_complex.h"

#endif /* TEST_GTY_H */

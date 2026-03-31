/* Primary test header for gengtype parsing coverage */
#ifndef TEST_GTY_H
#define TEST_GTY_H

/* Include secondary headers to test multi-file parsing */
#include "test_nested.h"
#include "test_macros.h"

/* Test 1: Complex function pointer with deeply nested parameter lists */
typedef int (*GTY((user)) complex_func_ptr)(
    int (*inner)(char[10][(sizeof(double) > 8) ? 5 : 10]),
    struct { 
        int a; 
        int b[(2+3)*4];
    } __attribute__((aligned(16)))
);

/* Test 2: Multi-dimensional array with parenthesized size expressions */
typedef int GTY((skip)) matrix_t[5][(sizeof(long long) > 8) ? 10 : 20][3];

/* Test 3: Nested union/struct with arrays and bit-fields */
typedef union GTY((chain_next("next"), chain_prev("prev"))) {
    struct GTY((tag("NODE_TYPE"))) {
        int x:8;
        char arr[5][(10/2)];
        long:0;  /* Zero-width bitfield */
    } s;
    long l;
    double d[(1+2)*3];
} nested_union_t;

/* Test 4: Function pointer returning another function pointer */
typedef void (*(*GTY((ptr_alias("func_factory"))))(
    int param1,
    char param2[][(sizeof(int)*2)]
))(
    struct { int a; double b; } __attribute__((packed))
);

/* Test 5: Nested typedef with GCC attributes containing balanced parens */
typedef int __attribute__((aligned(32), 
    deprecated("Use aligned_int64_t instead"))) 
    aligned_int_t[(sizeof(void*) == 8) ? 2 : 4];

/* Test 6: Pointer to array of function pointers */
typedef void (*(*GTY((user)) api_table[10])[
    (__SIZEOF_POINTER__ > 4) ? 2 : 1
])(
    int, 
    char *__restrict__
);

/* Include more complex types from secondary headers */
#include "test_attributes.h"

#endif /* TEST_GTY_H */

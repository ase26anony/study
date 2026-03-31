/* Primary test header for gengtype coverage testing */
#ifndef TEST_GTY_H
#define TEST_GTY_H

/* Include secondary headers to test multi-file parsing */
#include "test_nested.h"
#include "test_macros.h"

/* ==================== Complex Nested Type Definitions ==================== */

/* 1. Function pointer with deeply nested parameter lists */
typedef int (*complex_func_ptr)(
    int (*inner_callback)(char[10], int (*)(void)), 
    struct { 
        int a; 
        double b[5][(sizeof(long) > 4) ? 10 : 20]; 
    }
);

/* 2. Multi-dimensional array with parenthesized size expressions */
typedef int matrix_type[
    5][
    (sizeof(void*) == 8) ? 16 : 8
][
    (1 << 3) + 2
];

/* 3. Struct within union within typedef with arrays and bit-fields */
typedef union {
    struct {
        unsigned int flags : 4;
        char name[32];
        int matrix[3][(10 + 2)];
    } data;
    struct {
        long long id;
        union {
            float f;
            double d[(sizeof(double) / sizeof(float))];
        } value;
    } metadata;
    void *pointers[5];
} nested_union_t GTY((user));

/* ==================== GTY Annotation Stress Testing ==================== */

/* 4. GTY markers with complex nested token groups */
typedef GTY((user, desc("complex_type"))) 
struct node * GTY((skip, 
    chain_next("next"), 
    chain_prev("prev")
)) node_ptr;

/* 5. GTY with deeply nested attribute lists */
GTY((
    tag("LIST_TYPE"),
    length("count"),
    reorder("list_reorder")
)) struct list {
    struct list * GTY((skip)) next;
    struct list *prev;
    int data[10][(2 * 5)];
    union {
        char *str;
        int num;
        struct {
            short x;
            short y;
        } coords;
    } payload;
};

/* ==================== GCC Attributes with Balanced Groups ==================== */

/* 6. GCC attributes containing balanced parentheses */
typedef int __attribute__((
    aligned(16), 
    packed, 
    deprecated("Use 'aligned_int32_t' instead")
)) aligned_int;

/* 7. Function pointer with attributes */
typedef void (* __attribute__((
    stdcall,
    nonnull(1, 2),
    format(printf, 3, 4)
)) api_function)(
    int, 
    const char *, 
    ...
);

/* 8. Struct with packed attribute containing bit-fields */
typedef struct __attribute__((
    packed,
    aligned(4)
)) packed_struct {
    unsigned int a : 3;
    unsigned int b : 5;
    unsigned int c : 8;
    char arr[(10 + 2)];
} packed_struct_t;

#endif /* TEST_GTY_H */

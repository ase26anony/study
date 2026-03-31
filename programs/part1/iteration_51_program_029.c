/* test_gty.h - Primary header for gengtype coverage testing */

#ifndef TEST_GTY_H
#define TEST_GTY_H

/* Include secondary headers to test multi-file parsing */
#include "test_nested.h"
#include "test_macros.h"

/* ============================================
   Complex typedefs with deeply nested balanced tokens
   ============================================ */

/* 1. Function pointer with nested parameter lists and arrays */
typedef int (*complex_func_ptr_t)(
    int (*inner_callback)(char param[10]),
    struct { 
        int a; 
        double b[(sizeof(long) > 4) ? 8 : 4]; 
    } nested_struct
);

/* 2. Multi-dimensional array with parenthesized size expressions */
typedef int matrix_type_t[5][(sizeof(void*) * 2)][10];
typedef char string_array_t[(10 + 20) * 2][(sizeof(int) << 2)];

/* 3. Struct within union within typedef with GTY annotation */
typedef union GTY((desc("%1.a"))) {
    struct GTY((tag("STRUCT_A"))) {
        int x;
        char arr[5][(2 + 3)];
        long bitfield: (sizeof(int) * 8 - 4);
    } s;
    long GTY((skip)) l_array[10];
    void (*GTY((user)) fp)(int, char);
} nested_union_t GTY((user));

/* ============================================
   GTY annotations with complex nested arguments
   ============================================ */

/* GTY with nested parentheses in arguments */
typedef GTY((user, 
             ptr_alias("node_ptr_alias"),
             nested("struct tree_node *"))) 
        struct tree_node * GTY((skip)) tree_node_ptr;

/* GTY chain_next with array containing expression */
struct GTY((chain_next = "next", chain_prev = "prev")) linked_item {
    struct linked_item * GTY((skip)) next;
    struct linked_item * GTY((skip)) prev;
    int data[(10 * sizeof(void*)) + 2];
    char name[((5 | 3) & 7) + 1];
};

/* ============================================
   Attributes with balanced parentheses
   ============================================ */

/* GCC attributes containing balanced parentheses */
typedef int __attribute__((aligned(16), 
                          packed, 
                          deprecated("Use int32_t instead"))) 
        aligned_int_t;

/* Function pointer with stdcall attribute */
typedef void (__attribute__((stdcall)) *api_function_t)(
    int param1,
    char param2[(sizeof(int) + 3)],
    void (*callback)(void)
);

/* ============================================
   Complex nested type with all balanced tokens
   ============================================ */

typedef struct GTY((user)) {
    /* Nested struct with array */
    struct {
        int counters[5][(2 * 3)];
        union {
            char c;
            int i[((1 << 2) + 1)];
        } u;
    } inner;
    
    /* Function pointer array */
    int (*handlers[3])(
        char input[10],
        struct { int x; int y; } point
    );
    
    /* Pointer to array of pointers */
    int *(*(*complex_ptr)[10])[5];
    
    /* Bitfield with expression */
    unsigned int flags: (sizeof(int) * 8 - 1);
} ultimate_nested_t;

/* Include macro-based definitions */
#include "test_macros.h"

#endif /* TEST_GTY_H */

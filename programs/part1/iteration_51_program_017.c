/* Primary test header for gengtype parser coverage */
#ifndef TEST_GTY_H
#define TEST_GTY_H

#include "test_nested.h"
#include "test_macros.h"

/* Test 1: Complex function pointer with nested parameter lists */
typedef int (*complex_fp_type)(
    int (*inner_callback)(char[10]), 
    struct {int a; int b;} param
);

/* Test 2: GTY annotation with nested balanced tokens */
typedef GTY((user)) struct node * GTY((skip)) node_ptr;

/* Test 3: Multi-dimensional array with parenthesized size expressions */
typedef int matrix_t[5][(sizeof(long) > 4) ? 10 : 20];

/* Test 4: Nested union with struct containing arrays */
typedef union {
    struct {
        int x;
        char arr[5];
    } inner;
    long l;
    double d[(2+3)];
} nested_union_t GTY((tag("UNION")));

/* Test 5: Struct with chain_next/chain_prev and array with expression */
GTY((chain_next = "next", chain_prev = "prev")) 
struct linked_list {
    struct linked_list *next;
    struct linked_list *prev;
    int data[(10+2)];
    char buffer[sizeof(struct {int a; double b;})];
};

/* Test 6: Function pointer returning function pointer */
typedef void (*(*nested_fp_return)(int))(char);

/* Test 7: Array of function pointers */
typedef int (*fp_array_t[5])(int, ...);

/* Test 8: Struct with bit-fields and nested anonymous struct */
typedef struct {
    unsigned int flags:8;
    struct {
        int x:4;
        int y:4;
    } coord;
    int arr[3][2];
} bitfield_struct_t;

/* Test 9: GCC attributes with balanced parentheses */
typedef int __attribute__((aligned(16), packed)) aligned_int;
typedef void (*__attribute__((stdcall)) api_fn)(int, char*);

/* Test 10: Complex typedef with multiple levels of nesting */
typedef struct outer {
    union {
        int (*func_ptr)(int[2][2]);
        struct {
            char *name;
            int scores[5];
        } student;
    } data;
    struct outer *self_ref;
} outer_t GTY((user));

#endif /* TEST_GTY_H */

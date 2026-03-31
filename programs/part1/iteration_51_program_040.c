/* Primary test header for gengtype coverage testing */
#ifndef TEST_GTY_H
#define TEST_GTY_H

#include "test_nested.h"
#include "test_macros.h"

/* Complex typedef with deeply nested parentheses for function pointers */
typedef int (*complex_func_ptr_t)(int (*inner_callback)(char[10]), 
                                  struct {int a; int b;} param);

/* Multi-dimensional array with parenthesized size expressions */
typedef int matrix_t[5][(sizeof(long) > 4) ? 10 : 20][(2 + 3)];

/* Nested struct/union within typedef with arrays */
typedef union {
    struct {
        int x;
        char arr[5][(sizeof(int) * 2)];
    } inner;
    long l;
    double matrix[3][(4 + 1)];
} nested_union_t GTY((user));

/* GTY annotation with skip marker on complex pointer type */
typedef GTY((user)) struct node * GTY((skip)) node_ptr;

/* GTY with chain_next/chain_prev and nested array */
GTY((chain_next = "next", chain_prev = "prev")) 
struct linked_list {
    struct linked_list *next;
    struct linked_list *prev;
    int data[(10 + 2)][5];
    char *buffer;
};

/* Function pointer with GCC attributes containing balanced parentheses */
typedef void (__attribute__((stdcall, deprecated)) *api_function_t)(
    int param1,
    char *__attribute__((nonnull(1, 2))) param2[]
);

/* Complex nested type with multiple GTY markers */
typedef GTY((desc("%1.type"), tag("TYPE"))) 
struct type_info {
    int type;
    union {
        struct {
            int *array GTY((length("%h.dim[0] * %h.dim[1]"))) ;
            int dim[2];
        } matrix;
        struct {
            char *name;
            int id;
        } named;
    } u;
} *type_info_ptr GTY((skip));

/* Array of function pointers */
typedef int (*(*complex_array_t)[5])(void *arg, 
                                     struct { int x; int y; } point);

#endif /* TEST_GTY_H */

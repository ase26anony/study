/* test_gty.h - Primary header file for gengtype coverage testing */

#ifndef TEST_GTY_H
#define TEST_GTY_H

#include "test_nested.h"
#include "test_macros.h"

/* Complex typedef with deeply nested balanced tokens */
typedef int (*complex_func_ptr_t)(int (*inner)(char[10]), 
                                  struct {int a; int b;});

/* Multi-dimensional array with parenthesized size expression */
typedef int matrix_t[5][(sizeof(long) > 4) ? 10 : 20];

/* Struct within typedef containing arrays and bit-fields */
typedef union {
    struct {
        int x;
        char arr[5];
        unsigned int bitfield : 4;
    };
    long l;
    double d[(2+3)];
} nested_union_t GTY((user));

/* GTY annotation with nested tokens in arguments */
typedef GTY((user, (ptr_alias("node_ptr")))) struct node * GTY((skip)) node_ptr;

/* Chain structure with array in data field */
GTY((chain_next = "next", chain_prev = "prev")) 
struct list {
    struct list *next;
    struct list *prev;
    int data[(10+2)];
    char name[20][(sizeof(int)*2)];
};

/* Function pointer type with GTY marker */
typedef void (GTY((user)) *callback_t)(int, char **);

/* Nested structure with multiple balanced groups */
typedef struct GTY(()) outer_struct {
    struct {
        int *array_ptr[5];
        void (*func_array[3])(int, int);
    } inner;
    union {
        int i;
        char c[((8+2)/2)];
    } u;
} outer_struct_t;

/* Complex pointer to array of function pointers */
typedef int (*(*complex_array_ptr_t)[5])(char *, int[3]);

/* Include attribute specifications with balanced parentheses */
typedef int __attribute__((aligned(16), packed)) aligned_int;
typedef void (*__attribute__((stdcall)) api_fn)(int, 
                                                __attribute__((unused)) char);

/* Macro-based complex type */
typedef GTY(()) PTR_TO(NESTED_ARRAY) complex_ptr;

/* GTY with macro arguments */
typedef struct GTY_USER_ARGS my_struct {
    int i;
    double matrix[2][(4*2)];
} my_struct_t;

/* Deeply nested balanced tokens */
typedef struct {
    int (*((*func_ptr_array)[3]))(int, 
                                  struct { 
                                      char a[5]; 
                                      int b[2][2]; 
                                  });
} ultra_nested_t;

#endif /* TEST_GTY_H */

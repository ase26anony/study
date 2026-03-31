/* Primary test header for gengtype parsing coverage */
#ifndef TEST_GTY_H
#define TEST_GTY_H

#include "test_nested.h"
#include "test_macros.h"

/* Complex typedef with deeply nested parentheses for function pointers */
typedef int (*complex_func_ptr_t)(int (*inner)(char[10]), 
                                  struct {int a; int b;},
                                  void (*callback)(int, float));

/* Multi-dimensional array with parenthesized size expressions */
typedef int matrix_t[5][(sizeof(long) > 4) ? 10 : 20][(2+3)*4];

/* Struct within typedef containing arrays and nested structs */
typedef struct GTY((user)) {
    int data[10][(sizeof(int)*2)];
    struct {
        char name[50];
        int scores[(10+5)];
    } GTY((tag("student_data"))) student;
    union {
        long l;
        double d;
        char arr[(20+1)];
    } value;
} complex_struct_t;

/* GTY annotation with skip marker on complex pointer type */
typedef GTY((user)) struct node * GTY((skip)) node_ptr;

/* Function pointer type with GCC attributes */
typedef void (*__attribute__((stdcall, noreturn)) 
            api_function_t)(int param1, 
                           char param2[(sizeof(int)*4)]);

/* Chain-linked structure with GTY annotations */
GTY((chain_next = "next", chain_prev = "prev")) 
struct linked_list {
    struct linked_list *next;
    struct linked_list *prev;
    int data_items[(10+2)];
    char buffer[50][(5*2)];
};

/* Union containing struct with bit-fields */
typedef union GTY((desc("%0.type"))) {
    struct GTY((tag("0"))) {
        unsigned int type:4;
        unsigned int flags:8;
        int value;
        char name[(20+1)];
    } int_data;
    struct GTY((tag("1"))) {
        double dval;
        char str[100];
        int array[(3*3*3)];
    } float_data;
} variant_data_t;

/* Nested function pointer in struct */
typedef struct {
    int (*compare)(const void *a, const void *b, 
                   int (*secondary)(int, int));
    void (*cleanup)(struct { char *buf; int len; } *context);
} operations_t;

/* Include macro-based definitions */
#include "test_macros.h"

#endif /* TEST_GTY_H */

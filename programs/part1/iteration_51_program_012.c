/* Primary test header for gengtype parser coverage */
#ifndef TEST_GTY_H
#define TEST_GTY_H

#include "test_nested.h"
#include "test_macros.h"

/* Complex typedef with deeply nested parentheses */
typedef int (*complex_func_ptr)(
    int (*inner_callback)(char[10][(sizeof(double) > 8) ? 16 : 8]),
    struct {
        int a;
        long b[(10 + 2)];
    } __attribute__((packed, aligned(16)))
);

/* GTY annotation with nested token groups */
typedef GTY((user, 
            (ptr_alias("node_ptr")),
            (desc("tag @1 %h")),
            (param_is(struct node_tag))
           )) 
       struct node * GTY((skip)) node_ptr_array[5][(MAX_SIZE > 100) ? 10 : 5];

/* Multi-dimensional array with parenthesized size expressions */
typedef int matrix_t[
    5][
    (sizeof(long long) > 8) ? 20 : 10][
    (__alignof__(double) / 2)
];

/* Nested struct within typedef with GTY */
typedef GTY(()) struct outer_container {
    struct {
        GTY((chain_next = "next", chain_prev = "prev")) 
        struct inner_list {
            struct inner_list * GTY((tag("0"))) next;
            struct inner_list *prev;
            int data[5][(2 + 3)];
            union {
                float f;
                int i[4];
            } value;
        } *list_head;
    } container;
    
    enum {
        STATE_A,
        STATE_B = (1 << 2),
        STATE_C = STATE_B | 1
    } state;
    
    void (* GTY((skip)) operation)(
        int param1,
        char param2[(10)],
        struct {
            int x;
            int y;
        } point
    );
} outer_container_t;

/* Function pointer type with complex parameter list */
typedef void (*api_function_t)(
    int,
    char * GTY((length("%0"))),
    struct buffer {
        unsigned char data[1024];
        size_t size;
    } __attribute__((aligned(32))) *
);

/* Union containing struct with array */
typedef GTY((user)) union {
    struct {
        int x;
        char arr[5][10];
        struct {
            short s;
            long l;
        } nested;
    } data;
    long long as_int64;
    double as_double[2][(sizeof(void*) == 8) ? 4 : 2];
} nested_union_t;

/* GCC attributes with balanced parentheses */
typedef int __attribute__((aligned(16), 
                          packed,
                          deprecated("Use new_type instead"))) 
        aligned_int_t;

typedef void (* __attribute__((stdcall, 
                              noinline,
                              hot)) 
             winapi_fn)(int, char **);

/* Include another header with complex types */
#include "test_complex.h"

#endif /* TEST_GTY_H */

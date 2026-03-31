/* gty-test.h - Test file for gengtype type classification coverage */

#ifndef GTY_TEST_H
#define GTY_TEST_H

/* Forward declaration for TYPE_UNDEFINED */
struct opaque;

/* TYPE_STRUCT - Regular struct with GTY annotation */
struct GTY(()) my_struct {
    /* TYPE_SCALAR */
    int scalar_field;
    
    /* TYPE_STRING */
    const char * GTY((skip)) string_field;
    
    /* TYPE_POINTER */
    struct my_struct *next;
    
    /* TYPE_ARRAY - Fixed size array */
    int array_field[10];
    
    /* Pointer to undefined type - may influence TYPE_UNDEFINED */
    struct opaque *opaque_ptr;
};

/* TYPE_UNION */
union GTY(()) my_union {
    int as_int;
    float as_float;
    struct my_struct * GTY((tag("0"))) as_struct;
};

/* TYPE_USER_STRUCT - Using typedef with GTY */
typedef struct GTY(()) {
    int id;
    char *name;
} user_struct_t;

/* TYPE_CALLBACK - Function pointer typedef with GTY */
typedef void (* GTY((callback)) my_callback_t)(int, const char*);

/* Another struct using the callback type */
struct GTY(()) struct_with_callback {
    my_callback_t callback;
    user_struct_t user_data;
};

/* TYPE_LANG_STRUCT - Simulating language-specific structure */
struct GTY((chain_next ("%h.next"), chain_prev ("%h.prev"))) lang_struct {
    int lang_specific;
    struct lang_struct *next;
    struct lang_struct *prev;
};

/* Additional pointer types for coverage */
typedef struct my_struct * GTY(()) my_struct_ptr;
typedef union my_union * GTY(()) my_union_ptr;

/* Array of pointers */
struct GTY(()) array_container {
    /* TYPE_ARRAY of TYPE_POINTER */
    struct my_struct *ptr_array[5];
    
    /* Flexible array member */
    int flexible_array GTY((length("%h.dynamic_len")))[];
    int dynamic_len;
};

/* Nested structures for comprehensive coverage */
struct GTY(()) outer_struct {
    struct GTY(()) inner_struct {
        int inner_field;
    } inner;
    
    union GTY(()) inner_union {
        int a;
        double b;
    } u;
};

#endif /* GTY_TEST_H */

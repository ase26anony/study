/* gty-test.h - Test various GTY-annotated types for gengtype coverage */

#ifndef GTY_TEST_H
#define GTY_TEST_H

/* Forward declaration for undefined/opaque type */
struct opaque;

/* TYPE_STRUCT: Basic struct with GTY annotation */
struct GTY(()) my_struct {
    /* TYPE_SCALAR: Plain scalar member */
    int scalar_member;
    
    /* TYPE_STRING: String member */
    const char * GTY((skip)) string_member;
    
    /* TYPE_POINTER: Pointer to another struct */
    struct my_struct *next;
    
    /* TYPE_ARRAY: Array member */
    int array_member[10];
    
    /* TYPE_POINTER to opaque/undefined type */
    struct opaque *opaque_ptr;
};

/* TYPE_UNION: Union with GTY annotation */
union GTY(()) my_union {
    int as_int;
    float as_float;
    struct my_struct * GTY((tag("0"))) as_struct;
};

/* TYPE_CALLBACK: Function pointer typedef with GTY */
typedef void (* GTY((callback)) my_callback_t)(int, const char*);

/* Another struct using the callback type */
struct GTY(()) struct_with_callback {
    my_callback_t callback_func;
    union my_union data;
};

/* TYPE_USER_STRUCT: Using GTY markers for user-defined behavior */
struct user_defined {
    int id;
    char *name;
};

typedef struct user_defined * GTY((user)) user_defined_ptr_t;

/* Struct containing user-defined pointer */
struct GTY(()) container {
    user_defined_ptr_t user_ptr;
    int count;
};

/* Language-specific structure (simulating TYPE_LANG_STRUCT) */
struct GTY((desc("%1.type"), tag("0"))) lang_type {
    int type;
    union {
        int basic_type;
        struct my_struct *complex_type;
    } GTY((desc("%0.type"))) u;
};

/* Array of pointers */
typedef struct my_struct * GTY(()) ptr_array_t[5];

/* Nested structure for complex testing */
struct GTY(()) outer_struct {
    struct GTY(()) inner_struct {
        int value;
        struct inner_struct *self_ptr;
    } inner;
    
    /* Array of unions */
    union my_union union_array[3];
    
    /* Pointer to array */
    int (*matrix_ptr)[4][4];
};

#endif /* GTY_TEST_H */

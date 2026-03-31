/* gty-test.h - Test file for gengtype type categorization */
#ifndef GTY_TEST_H
#define GTY_TEST_H

/* Forward declaration for undefined/opaque type */
struct opaque;

/* TYPE_UNDEFINED: Forward-declared opaque struct pointer */
typedef struct opaque *opaque_ptr_t;

/* TYPE_CALLBACK: Function pointer type */
typedef void (*callback_t)(int, const char *) GTY((callback));

/* TYPE_STRUCT: Basic struct with GTY annotation */
struct GTY(()) basic_struct {
    /* TYPE_SCALAR: Plain scalar member */
    int scalar_member;
    
    /* TYPE_STRING: String member */
    const char * GTY((length("strlen(%h.string_member) + 1"))) string_member;
    
    /* TYPE_POINTER: Pointer to another struct */
    struct basic_struct *next;
    
    /* TYPE_ARRAY: Array member */
    int GTY((length("array_length"))) array_member[10];
    
    /* TYPE_UNDEFINED: Opaque pointer */
    struct opaque *opaque_ptr;
    
    /* TYPE_CALLBACK: Function pointer field */
    callback_t callback_field;
};

/* TYPE_USER_STRUCT: Typedef'd struct with GTY */
typedef struct GTY(()) user_struct {
    int data;
    struct basic_struct *link;
} user_struct_t;

/* TYPE_UNION: Union with GTY annotation */
union GTY(()) test_union {
    int as_int;
    float as_float;
    struct basic_struct * GTY((tag("0"))) as_struct;
    user_struct_t * GTY((tag("1"))) as_user;
};

/* TYPE_LANG_STRUCT: Language-specific structure */
struct GTY((chain_next ("%h.next"), chain_prev ("%h.prev"))) lang_struct {
    struct lang_struct *next;
    struct lang_struct *prev;
    int lang_specific_data;
};

/* Another struct with nested pointer/array combinations */
struct GTY(()) complex_struct {
    /* Array of pointers (TYPE_ARRAY of TYPE_POINTER) */
    struct basic_struct * GTY((length("ptr_count"))) ptr_array[5];
    
    /* Pointer to array (TYPE_POINTER to TYPE_ARRAY) */
    int (*matrix_ptr)[3][3];
    
    /* Union member */
    union test_union union_member;
    
    /* Callback array */
    callback_t GTY((length("callback_count"))) callbacks[3];
};

/* Global variable declarations for gengtype to process */
extern struct basic_struct * GTY((root)) global_root;
extern user_struct_t * GTY((root)) user_root;
extern union test_union GTY((root)) global_union;

#endif /* GTY_TEST_H */

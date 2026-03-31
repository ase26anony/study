/* gengtype_test_types.h - Comprehensive type definitions for gengtype coverage */
#ifndef GTY_TEST_TYPES_H
#define GTY_TEST_TYPES_H

/* Include gtype-desc.h for GTY macro if not already defined */
#ifndef GTY
#define GTY(x) x
#endif

/* TYPE_SCALAR: Basic typedefs */
typedef int my_int;
typedef unsigned int my_uint;
typedef char my_char;
typedef double my_double;

/* TYPE_STRING: String type definitions */
typedef const char *string_t;
typedef char *mutable_string_t;

/* TYPE_STRUCT: Plain C structs (not GTY-tagged) */
struct plain_struct {
    int field1;
    double field2;
};

/* TYPE_USER_STRUCT: GTY-tagged structs */
struct GTY(()) user_struct {
    int id;
    struct plain_struct *plain_ptr;  /* Pointer to non-GTY struct */
    struct user_struct *next;        /* Recursive pointer */
};

/* TYPE_UNION: Union definitions */
union my_union {
    int int_val;
    double double_val;
    void *ptr_val;
};

/* TYPE_POINTER: Various pointer types */
typedef struct user_struct *user_ptr_t;
typedef void *generic_ptr_t;
typedef int *int_ptr_t;

/* TYPE_ARRAY: Array types within structs */
struct GTY(()) array_container {
    int fixed_array[10];
    struct user_struct *ptr_array[5];
    char string_array[3][20];
};

/* TYPE_CALLBACK: Function pointer types */
typedef void (*simple_callback)(int);
typedef int (*complex_callback)(struct user_struct *, string_t);
typedef void (*void_callback)(void);

/* TYPE_LANG_STRUCT: Language-specific structs */
#ifdef GENERATOR_FILE
struct GTY(()) lang_specific_struct {
    int generator_only_field;
    struct user_struct *gty_link;
};
#endif

/* More complex nested structures to ensure deep traversal */

/* Union containing GTY-tagged pointer */
union GTY(()) tagged_union {
    struct user_struct * GTY((tag("0"))) user_ptr;
    struct array_container * GTY((tag("1"))) array_ptr;
    int int_value;
};

/* Struct with callback field */
struct GTY(()) callback_container {
    int id;
    simple_callback cb;
    complex_callback complex_cb;
};

/* Struct with nested arrays and pointers */
struct GTY(()) complex_nested {
    int matrix[3][3];
    struct callback_container *callbacks[4];
    union my_union variants[2];
    string_t names[5];
};

/* Recursive structure for deep traversal */
struct GTY(()) tree_node {
    int value;
    struct tree_node * GTY((skip)) left;    /* Skip this for GC */
    struct tree_node *right;
    struct tree_node *parent;
};

/* Another user struct with different GTY options */
struct GTY((chain_next ("%h.next"), chain_prev ("%h.prev"))) linked_node {
    int data;
    struct linked_node *next;
    struct linked_node *prev;
    string_t name;
};

/* Mixed struct with all kinds of fields */
struct GTY(()) mixed_bag {
    /* Scalar fields */
    my_int count;
    my_double value;
    
    /* String field */
    string_t description;
    
    /* Pointer fields */
    struct user_struct *user;
    generic_ptr_t generic;
    
    /* Array fields */
    int scores[5];
    struct user_struct *users[3];
    
    /* Union field */
    union my_union data;
    
    /* Callback field */
    void_callback cleanup;
    
    /* Nested struct */
    struct plain_struct plain;
};

/* Forward declaration to test TYPE_UNDEFINED handling */
struct forward_declared;

/* Now define it */
struct GTY(()) forward_declared {
    int defined_now;
    struct forward_declared *next;
};

/* Enumeration (should be treated as scalar) */
typedef enum {
    STATE_INIT,
    STATE_RUNNING,
    STATE_DONE
} process_state_t;

/* Struct containing enum */
struct GTY(()) state_container {
    process_state_t state;
    string_t state_name;
};

#endif /* GTY_TEST_TYPES_H */

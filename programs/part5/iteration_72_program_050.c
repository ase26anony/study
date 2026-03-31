#ifndef GTY_TEST_TYPES_H
#define GTY_TEST_TYPES_H

#ifdef __cplusplus
extern "C" {
#endif

/* Basic structure type */
struct GTY(()) base_struct {
    int id;
    char *GTY((skip)) name;  /* Skip this pointer */
};

/* Union type */
union GTY(()) data_union {
    int int_val;
    float float_val;
    void *GTY((skip)) ptr_val;
    struct base_struct *GTY((skip)) struct_ptr;
};

/* Recursive structure (linked list) */
struct GTY(()) list_node {
    int data;
    struct list_node *GTY((skip)) next;  /* Pointer to same type */
    struct list_node *GTY((skip)) prev;  /* Another pointer */
};

/* Array type definition */
typedef int GTY(()) int_array[10];
typedef struct base_struct *GTY(()) struct_ptr_array[5];

/* Structure containing union */
struct GTY(()) container {
    union data_union GTY((tag("0"))) data;  /* Union with tag option */
    int type;
};

/* User-defined struct type via typedef */
typedef struct base_struct GTY(()) base_struct_t;

/* Callback (function pointer) type */
typedef void (*GTY(()) callback_func)(int, void*);

/* Structure with callback */
struct GTY(()) event_handler {
    callback_func GTY((skip)) handler;
    void *GTY((skip)) user_data;
};

/* Nested structure */
struct GTY(()) outer_struct {
    struct GTY(()) inner_struct {
        int x;
        int y;
    } inner;
    struct outer_struct *GTY((skip)) parent;
};

/* For TYPE_LANG_STRUCT simulation - using a special name pattern */
struct GTY(()) lang_specific {
    int lang_data;
    void *GTY((skip)) lang_ptr;
};

/* Scalar type in container */
struct GTY(()) scalar_container {
    long GTY((skip)) scalar_value;
    double GTY((skip)) double_value;
};

/* String type handling */
struct GTY(()) string_container {
    const char *GTY((skip)) constant_string;
    char *GTY((skip)) dynamic_string;
};

/* Array with length option */
struct GTY(()) variable_array {
    int count;
    int *GTY((length("count"))) items;  /* Array with length specifier */
};

/* Multiple GTY options chained */
struct GTY(()) complex_options {
    void *GTY((skip, tag("1"))) special_ptr;
    int GTY((skip)) ignored_field;
};

#ifdef USE_GTY
struct GTY(()) conditional_type {
    int conditional_field;
    struct conditional_type *GTY((skip)) next;
};
#else
struct conditional_type {
    int conditional_field;
    struct conditional_type *next;
};
#endif

/* Forward declaration for mutual recursion */
struct GTY(()) forward_decl;
struct GTY(()) mutual_a;
struct GTY(()) mutual_b;

/* Complete the mutual recursion */
struct GTY(()) mutual_a {
    int a_data;
    struct mutual_b *GTY((skip)) b_ptr;
};

struct GTY(()) mutual_b {
    int b_data;
    struct mutual_a *GTY((skip)) a_ptr;
};

/* Opaque pointer type */
typedef struct forward_decl *GTY(()) opaque_ptr_t;

#ifdef __cplusplus
}
#endif

#endif /* GTY_TEST_TYPES_H */

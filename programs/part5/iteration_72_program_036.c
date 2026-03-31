#ifndef GTY_TEST_TYPES_H
#define GTY_TEST_TYPES_H

/* Basic structure type with GTY marker */
struct GTY(()) base_struct {
    int id;
    char *GTY((skip)) name;  /* Skip this field */
};

/* Union type */
union GTY(()) data_union {
    int int_val;
    float float_val;
    void *GTY((skip)) ptr_val;
    struct base_struct *struct_ptr;
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

/* User-defined struct type via typedef */
typedef struct base_struct GTY(()) base_struct_t;

/* Callback (function pointer) type */
typedef void (*GTY(()) callback_func)(int, void*);

/* Structure containing a callback */
struct GTY(()) callback_container {
    callback_func GTY((skip)) handler;
    void *GTY((skip)) user_data;
};

/* Nested structure with union */
struct GTY(()) complex_nested {
    union data_union data;
    struct {
        int x;
        int y;
    } GTY((skip)) coordinates;
    struct list_node *GTY((skip)) node_list;
};

/* Scalar type in container */
struct GTY(()) scalar_box {
    long GTY((skip)) value;
    unsigned long GTY((skip)) flags;
};

/* Conditional compilation for GTY */
#ifdef USE_GTY_MARKERS
struct GTY(()) conditional_struct {
    int conditional_field;
    char *GTY((skip)) conditional_string;
};
#else
struct conditional_struct {
    int conditional_field;
    char *conditional_string;
};
#endif

/* Multiple GTY options chained */
struct GTY(()) multi_option_struct {
    int *GTY((skip, tag("special_ptr"))) special_pointer;
    char **GTY((skip, length("str_len"))) string_array;
    int str_len;
};

/* Forward declaration for mutual recursion */
struct GTY(()) forward_decl_struct;

/* Complete the mutual recursion */
struct GTY(()) mutual_struct_a {
    int id;
    struct forward_decl_struct *GTY((skip)) partner;
};

struct GTY(()) forward_decl_struct {
    int value;
    struct mutual_struct_a *GTY((skip)) other;
};

/* Array of structures */
struct GTY(()) array_container {
    struct base_struct elements[4];
    int count;
};

/* Simulating lang_struct-like behavior with special tag */
struct GTY((tag("LANG_STRUCT"))) lang_like_struct {
    int lang_specific;
    void *GTY((skip)) lang_data;
};

/* External declarations for cross-file testing */
extern struct base_struct GTY(()) external_struct;
extern union data_union GTY(()) external_union;

#endif /* GTY_TEST_TYPES_H */

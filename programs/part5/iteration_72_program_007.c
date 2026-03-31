#ifndef GTY_TEST_TYPES_H
#define GTY_TEST_TYPES_H

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
    struct list_node *GTY((skip)) next;
    struct list_node *GTY((skip)) prev;
};

/* Array type definition */
typedef int GTY(()) int_array[10];
typedef struct base_struct *GTY(()) struct_ptr_array[5];

/* Callback (function pointer) type */
typedef void (*GTY(()) callback_func)(int, void*);

/* User-defined struct type via typedef */
typedef struct base_struct GTY(()) base_struct_t;

/* Nested structure containing union */
struct GTY(()) container {
    int type;
    union data_union GTY((skip)) data;
    struct list_node *GTY((skip)) node_list;
};

/* Conditional compilation for GTY */
#ifdef USE_GTY_MARKERS
struct GTY(()) conditional_struct {
    int x;
    double y;
    char *GTY((skip)) z;
};
#else
struct conditional_struct {
    int x;
    double y;
    char *z;
};
#endif

/* Structure with array field using length option */
struct GTY(()) array_container {
    int count;
    int *GTY((length("count"))) items;  /* Variable length array */
};

/* Complex nested type */
struct GTY(()) complex_type {
    struct container *GTY((skip)) container_ptr;
    union data_union current_union;
    callback_func GTY((skip)) handler;
    int_array fixed_array;
};

/* Forward declaration for mutual recursion */
struct GTY(()) tree_node;
struct GTY(()) tree_node {
    int value;
    struct tree_node *GTY((skip)) left;
    struct tree_node *GTY((skip)) right;
    struct tree_node *GTY((skip)) parent;
};

/* Simulating lang_struct pattern - often used in GCC internals */
struct GTY(()) lang_type {
    struct GTY((tag("0"))) lang_type_base {
        int code;
    } base;
    union GTY((desc("(%1.base.code)"))) lang_type_u {
        struct GTY((tag("1"))) lang_struct_data {
            int field1;
            char *GTY((skip)) field2;
        } s;
        struct GTY((tag("2"))) lang_union_data {
            float f;
            double d;
        } u;
    } data;
};

/* String type usage */
struct GTY(()) string_container {
    const char *GTY((skip)) message;
    char *GTY((skip)) buffer;
};

/* Scalar type in container */
struct GTY(()) scalar_box {
    long GTY((skip)) long_value;
    unsigned GTY((skip)) flags;
    _Bool GTY((skip)) boolean_flag;
};

/* Multiple GTY options chained */
struct GTY(()) option_test {
    void *GTY((skip, tag("1"))) ptr1;
    void *GTY((skip, tag("2"))) ptr2;
    int GTY((skip)) regular_field;
};

#endif /* GTY_TEST_TYPES_H */

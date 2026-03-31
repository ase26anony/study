#ifndef TYPES_H
#define TYPES_H

#ifdef __cplusplus
extern "C" {
#endif

/* Basic structure type */
struct GTY(()) my_struct {
    int a;
    char *GTY((skip)) b;  /* Pointer with skip option */
};

/* Union type */
union GTY(()) my_union {
    int i;
    float f;
    void *GTY((skip)) p;
};

/* Recursive structure for TYPE_POINTER handling */
struct GTY(()) list_node {
    struct list_node *GTY((skip)) next;
    struct list_node *GTY((skip)) prev;
    int data;
};

/* Array type definition */
typedef int GTY(()) int_array[10];

/* User-defined struct type via typedef */
typedef struct my_struct GTY(()) my_struct_t;

/* Callback function pointer type */
typedef void (*GTY(()) callback_fn)(int);

/* Structure containing union */
struct GTY(()) struct_with_union {
    int type;
    union GTY(()) {
        int int_val;
        float float_val;
        char *GTY((skip)) str_val;
    } value;
};

/* Union containing structure */
union GTY(()) union_with_struct {
    struct GTY(()) {
        int x;
        int y;
    } point;
    struct GTY(()) {
        float r;
        float g;
        float b;
    } color;
};

/* For TYPE_LANG_STRUCT simulation - using special naming pattern */
struct GTY(()) lang_struct_sim {
    int lang_specific;
    void *GTY((skip)) lang_data;
};

/* Scalar type in container */
struct GTY(()) scalar_box {
    long GTY((skip)) value;
    unsigned long GTY((skip)) uvalue;
};

/* Multi-dimensional array */
struct GTY(()) matrix {
    int GTY(()) data[4][4];
};

/* Conditional compilation with GTY */
#ifdef USE_GTY
struct GTY(()) conditional_struct {
    int x;
    double y;
    char *GTY((skip)) name;
};
#else
struct conditional_struct {
    int x;
    double y;
    char *name;
};
#endif

/* Chain multiple GTY options */
struct GTY(()) options_test {
    int *GTY((skip, tag("skip_and_tag"))) ptr1;
    void *GTY((skip)) ptr2;
};

/* Array of pointers to different types */
struct GTY(()) pointer_array {
    struct my_struct *GTY((skip)) struct_ptrs[5];
    union my_union *GTY((skip)) union_ptrs[3];
    callback_fn GTY((skip)) callbacks[2];
};

/* Nested type complexity */
struct GTY(()) outer_struct {
    struct GTY(()) inner {
        int depth;
        struct outer_struct *GTY((skip)) parent;
    } inner_obj;
    
    union GTY(()) choice {
        int as_int;
        struct inner *GTY((skip)) as_inner;
    } selection;
    
    int_array numbers;
};

/* Forward declaration for mutual recursion */
struct GTY(()) tree_node;
struct GTY(()) tree_node {
    int value;
    struct tree_node *GTY((skip)) left;
    struct tree_node *GTY((skip)) right;
    struct tree_node *GTY((skip)) parent;
};

/* String type handling */
struct GTY(()) string_container {
    const char *GTY((skip)) constant_str;
    char *GTY((skip)) mutable_str;
};

/* Function pointer with complex signature */
typedef int (*GTY(()) complex_callback)(
    struct my_struct *GTY((skip)),
    union my_union *GTY((skip)),
    callback_fn GTY((skip))
);

/* Global variable declarations with GTY */
extern struct my_struct GTY(()) global_struct;
extern union my_union GTY(()) global_union;
extern struct list_node *GTY((skip)) global_list;

#ifdef __cplusplus
}
#endif

#endif /* TYPES_H */

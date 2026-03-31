#ifndef GTY_TEST_TYPES_H
#define GTY_TEST_TYPES_H

/* Basic structure type - TYPE_STRUCT */
struct GTY(()) base_struct {
    int GTY((skip)) id;
    char *GTY((skip)) name;
};

/* Union type - TYPE_UNION */
union GTY(()) data_union {
    int GTY((skip)) int_val;
    float GTY((skip)) float_val;
    void *GTY((skip)) ptr_val;
};

/* Pointer type within structure - TYPE_POINTER */
struct GTY(()) list_node {
    struct list_node *GTY((skip)) next;
    struct list_node *GTY((skip)) prev;
    void *GTY((skip)) data;
};

/* Array type - TYPE_ARRAY */
typedef int GTY(()) int_array[10];
typedef struct base_struct GTY(()) struct_array[5];

/* User-defined type via typedef - TYPE_USER_STRUCT */
typedef struct base_struct GTY(()) base_struct_t;
typedef union data_union GTY(()) data_union_t;

/* Callback type - TYPE_CALLBACK */
typedef void (*GTY(()) callback_fn)(int, void*);
typedef int (*GTY(()) compare_fn)(const void*, const void*);

/* Scalar type in container - TYPE_SCALAR */
struct GTY(()) scalar_container {
    long GTY((skip)) long_val;
    double GTY((skip)) double_val;
    unsigned GTY((skip)) flags;
};

/* String type - TYPE_STRING */
struct GTY(()) string_container {
    const char *GTY((skip)) message;
    char *GTY((skip)) buffer;
};

/* Nested structure with union */
struct GTY(()) complex_struct {
    struct base_struct GTY((skip)) base;
    union data_union GTY((skip)) data;
    struct list_node *GTY((skip)) head;
    callback_fn GTY((skip)) handler;
};

/* Recursive type structure */
struct GTY(()) tree_node {
    int GTY((skip)) value;
    struct tree_node *GTY((skip)) left;
    struct tree_node *GTY((skip)) right;
    struct tree_node *GTY((skip)) parent;
};

/* Array of pointers */
struct GTY(()) pointer_array_container {
    struct base_struct *GTY((skip)) struct_ptrs[8];
    union data_union *GTY((skip)) union_ptrs[4];
    callback_fn GTY((skip)) callbacks[3];
};

/* Conditional compilation with GTY */
#ifdef USE_GTY_MARKERS
struct GTY(()) conditional_struct {
    int GTY((skip)) x;
    char *GTY((skip)) y;
};
#else
struct conditional_struct {
    int x;
    char *y;
};
#endif

/* Multiple GTY options */
struct GTY(()) options_struct {
    /* Chain multiple options */
    struct tree_node *GTY((skip, tag("tree"))) root;
    int GTY((length("len"))) *GTY((skip)) variable_array;
    int GTY((skip)) len;
    void *GTY((skip)) GTY((desc("tag"))) tagged_ptr;
};

/* Simulating lang_struct - using a naming convention that might be special */
struct GTY(()) lang_struct_sim {
    int GTY((skip)) lang_specific;
    void *GTY((skip)) lang_data;
};

/* Forward declarations with GTY */
struct GTY(()) forward_declared;
typedef struct forward_declared GTY(()) forward_declared_t;

/* Opaque pointer type */
typedef struct GTY(()) opaque_struct *GTY(()) opaque_ptr;

#endif /* GTY_TEST_TYPES_H */

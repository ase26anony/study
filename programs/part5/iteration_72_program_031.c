#ifndef GTY_TEST_TYPES_H
#define GTY_TEST_TYPES_H

#ifdef __cplusplus
extern "C" {
#endif

/* Basic structure type - TYPE_STRUCT */
struct GTY(()) base_struct {
    int id;
    char *GTY((skip)) name;  /* Pointer with skip option */
};

/* Union type - TYPE_UNION */
union GTY(()) data_union {
    int int_val;
    float float_val;
    void *GTY((skip)) ptr_val;
    struct base_struct *GTY((skip)) struct_ptr;
};

/* Recursive structure for TYPE_POINTER traversal */
struct GTY(()) tree_node {
    int value;
    struct tree_node *GTY((skip)) left;
    struct tree_node *GTY((skip)) right;
    union data_union GTY((skip)) data;
};

/* Array type definition - TYPE_ARRAY */
typedef int GTY(()) int_array[10];
typedef struct tree_node *GTY(()) node_ptr_array[5];

/* Callback type - TYPE_CALLBACK */
typedef void (*GTY(()) callback_func)(int, const char*);
typedef int (*GTY(()) compare_func)(const void*, const void*);

/* User-defined struct type via typedef - TYPE_USER_STRUCT */
typedef struct base_struct GTY(()) base_struct_t;

/* String type - TYPE_STRING */
struct GTY(()) string_container {
    const char *GTY((skip)) str;  /* String pointer */
    int length;
};

/* Scalar type in container - TYPE_SCALAR */
struct GTY(()) scalar_box {
    long GTY((skip)) scalar_value;
    unsigned long GTY((skip)) flags;
};

/* Complex nested structure */
struct GTY(()) complex_container {
    struct base_struct GTY((skip)) base;
    union data_union GTY((skip)) union_field;
    int_array GTY((skip)) numbers;
    struct complex_container *GTY((skip)) next;
    callback_func GTY((skip)) callback;
};

/* Conditional compilation with GTY */
#ifdef USE_GTY_OPTIONS
struct GTY((chain_next("next"), chain_prev("prev"))) linked_node {
    int data;
    struct linked_node *next;
    struct linked_node *prev;
};
#else
struct linked_node {
    int data;
    struct linked_node *next;
    struct linked_node *prev;
};
#endif

/* Multiple GTY options chained */
struct GTY(()) option_test {
    int GTY((skip, tag("0"))) tagged_field;
    void *GTY((skip, desc("1"))) described_ptr;
    struct tree_node **GTY((length("len"))) nodes;
    int len;
};

/* Simulating lang_struct pattern - TYPE_LANG_STRUCT */
/* In GCC, lang_struct types often have special handling */
struct GTY(()) lang_simul {
    int lang_specific;
    void *GTY((skip)) lang_data;
};

/* Forward declaration for mutual recursion */
struct GTY(()) forward_decl;
struct GTY(()) container_a;
struct GTY(()) container_b;

/* Mutual recursion structures */
struct GTY(()) container_a {
    int type;
    struct container_b *GTY((skip)) partner;
    struct forward_decl *GTY((skip)) future;
};

struct GTY(()) container_b {
    int id;
    struct container_a *GTY((skip)) owner;
    callback_func GTY((skip)) handler;
};

struct GTY(()) forward_decl {
    struct container_a *GTY((skip)) source;
    struct container_b *GTY((skip)) target;
};

#ifdef __cplusplus
}
#endif

#endif /* GTY_TEST_TYPES_H */

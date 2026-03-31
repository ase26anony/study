#ifndef GTY_TYPES_H
#define GTY_TYPES_H

/* Basic GTY-marked structure - TYPE_STRUCT */
struct GTY(()) base_struct {
    int GTY((skip)) id;
    char *GTY((skip)) name;
};

/* GTY-marked union - TYPE_UNION */
union GTY(()) data_union {
    int GTY((skip)) int_val;
    float GTY((skip)) float_val;
    void *GTY((skip)) ptr_val;
    struct base_struct *GTY((skip)) struct_ptr;
};

/* Pointer type within structure - TYPE_POINTER */
struct GTY(()) list_node {
    struct list_node *GTY((skip)) next;
    struct list_node *GTY((skip)) prev;
    union data_union GTY((skip)) data;
};

/* Array type definition - TYPE_ARRAY */
typedef int GTY(()) int_array[10];
typedef struct list_node *GTY(()) node_ptr_array[20];

/* Typedef creating user-defined type - TYPE_USER_STRUCT */
typedef struct base_struct GTY(()) base_struct_t;
typedef union data_union GTY(()) data_union_t;

/* Function pointer (callback) type - TYPE_CALLBACK */
typedef void (*GTY(()) callback_fn)(int, void*);
typedef int (*GTY(()) compare_fn)(const void*, const void*);

/* Scalar type in container - TYPE_SCALAR */
struct GTY(()) scalar_container {
    long GTY((skip)) counter;
    unsigned GTY((skip)) flags;
    size_t GTY((skip)) size;
};

/* String type - TYPE_STRING */
struct GTY(()) string_wrapper {
    const char *GTY((skip)) str;
    char *GTY((skip)) mutable_str;
};

/* Nested structure with multiple type kinds */
struct GTY(()) complex_type {
    struct base_struct GTY((skip)) base;
    union data_union GTY((skip)) union_field;
    struct list_node *GTY((skip)) head;
    int_array GTY((skip)) numbers;
    callback_fn GTY((skip)) handler;
    struct scalar_container GTY((skip)) scalars;
};

/* Conditional compilation with GTY */
#ifdef USE_GTY_OPTIONS
struct GTY((chain_next("next"), chain_prev("prev"))) linked_node {
    struct linked_node *next;
    struct linked_node *prev;
    int data;
};
#else
struct linked_node {
    struct linked_node *next;
    struct linked_node *prev;
    int data;
};
#endif

/* Recursive type structure */
struct GTY(()) tree_node {
    int GTY((skip)) value;
    struct tree_node *GTY((skip)) left;
    struct tree_node *GTY((skip)) right;
    struct tree_node *GTY((skip)) parent;
};

/* Union containing structures */
union GTY(()) variant_data {
    struct base_struct GTY((skip)) as_struct;
    struct scalar_container GTY((skip)) as_scalar;
    struct string_wrapper GTY((skip)) as_string;
};

/* Array of different GTY-marked types */
struct GTY(()) heterogeneous_array {
    struct base_struct *GTY((skip)) structs[5];
    union data_union GTY((skip)) unions[5];
    callback_fn GTY((skip)) callbacks[3];
};

/* For TYPE_LANG_STRUCT simulation - using naming convention */
struct GTY(()) lang_decl {
    struct base_struct GTY((skip)) common;
    void *GTY((skip)) lang_specific;
};

struct GTY(()) lang_type {
    struct scalar_container GTY((skip)) common;
    void *GTY((skip)) lang_info;
};

/* Multiple GTY options chained */
struct GTY(()) options_test {
    int *GTY((skip, tag("0"))) ptr1;
    char **GTY((skip, length("len"))) strings;
    int len;
};

/* Forward declarations with GTY */
struct GTY(()) forward_declared;
typedef struct forward_declared GTY(()) forward_declared_t;

#endif /* GTY_TYPES_H */

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
    struct base_struct *GTY((skip)) struct_ptr;
};

/* Pointer type within structure - TYPE_POINTER */
struct GTY(()) list_node {
    struct list_node *GTY((skip)) next;
    struct list_node *GTY((skip)) prev;
    union data_union GTY((skip)) data;
};

/* Array type - TYPE_ARRAY */
typedef struct base_struct GTY(()) struct_array[10];
typedef union data_union GTY(()) union_array[5];

/* User-defined struct type - TYPE_USER_STRUCT */
typedef struct base_struct GTY(()) base_struct_t;
typedef struct list_node GTY(()) list_node_t;

/* Callback type - TYPE_CALLBACK */
typedef void (*GTY(()) callback_func)(int, void*);
typedef int (*GTY(()) compare_func)(const void*, const void*);

/* String type - TYPE_STRING */
typedef const char *GTY(()) const_string;
typedef char *GTY((skip)) mutable_string;

/* Scalar type container - TYPE_SCALAR */
struct GTY(()) scalar_container {
    long GTY((skip)) long_val;
    unsigned GTY((skip)) uint_val;
    double GTY((skip)) double_val;
};

/* Complex nested structure */
struct GTY(()) complex_struct {
    struct base_struct GTY((skip)) base;
    union data_union GTY((skip)) data;
    struct list_node *GTY((skip)) head;
    struct list_node *GTY((skip)) tail;
    callback_func GTY((skip)) callback;
    struct_array GTY((skip)) array;
};

/* Conditional compilation with GTY */
#ifdef USE_GTY_MARKERS
struct GTY(()) conditional_struct {
    int GTY((skip)) x;
    double GTY((skip)) y;
};
#else
struct conditional_struct {
    int x;
    double y;
};
#endif

/* GTY with multiple options */
struct GTY(()) multi_option_struct {
    int GTY((skip, tag("0"))) tag_field;
    void *GTY((skip, desc("1"))) desc_field;
    struct list_node *GTY((chain_next("next"), chain_prev("prev"))) chain_node;
};

/* Recursive type definition */
struct GTY(()) tree_node {
    int GTY((skip)) value;
    struct tree_node *GTY((skip)) left;
    struct tree_node *GTY((skip)) right;
    struct tree_node *GTY((skip)) parent;
};

/* Array of pointers */
typedef struct tree_node *GTY(()) node_ptr_array[20];

/* Union containing structure pointer */
union GTY(()) container_union {
    struct base_struct *GTY((skip)) base_ptr;
    struct complex_struct *GTY((skip)) complex_ptr;
    struct tree_node *GTY((skip)) tree_ptr;
};

/* For TYPE_LANG_STRUCT simulation - using a naming convention 
   that might trigger special handling in gengtype */
struct GTY(()) lang_decl {
    int GTY((skip)) lang_specific;
    void *GTY((skip)) lang_data;
};

/* Typedef for lang struct */
typedef struct lang_decl GTY(()) lang_decl_t;

/* Function pointer with complex signature */
typedef struct base_struct *(*GTY(()) factory_func)(int, const char*);

/* Self-referential union */
union GTY(()) self_ref_union {
    int GTY((skip)) int_member;
    union self_ref_union *GTY((skip)) union_ptr;
};

#endif /* GTY_TEST_TYPES_H */

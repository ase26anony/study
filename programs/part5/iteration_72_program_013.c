#ifndef GTY_TEST_TYPES_H
#define GTY_TEST_TYPES_H

#ifdef __cplusplus
extern "C" {
#endif

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
struct GTY(()) linked_node {
    struct linked_node *GTY((skip)) next;
    struct linked_node *GTY((skip)) prev;
    union data_union GTY((skip)) data;
};

/* Array type - TYPE_ARRAY */
typedef int GTY(()) int_array[10];
typedef struct base_struct *GTY(()) struct_ptr_array[5];

/* User-defined struct type - TYPE_USER_STRUCT */
typedef struct base_struct GTY(()) base_struct_t;
typedef union data_union GTY(()) data_union_t;

/* Callback type - TYPE_CALLBACK */
typedef void (*GTY(()) callback_func)(int, void *);
typedef int (*GTY(()) compare_func)(const void *, const void *);

/* String type - TYPE_STRING */
typedef const char *GTY(()) const_string;

/* Scalar type container - TYPE_SCALAR */
struct GTY(()) scalar_container {
    long GTY((skip)) scalar1;
    unsigned long long GTY((skip)) scalar2;
    double GTY((skip)) scalar3;
};

/* Complex nested structure */
struct GTY(()) complex_nested {
    struct linked_node *GTY((skip)) node_list;
    union data_union GTY((skip)) variant;
    int_array GTY((skip)) numbers;
    callback_func GTY((skip)) handler;
};

/* Self-referential structure for recursion */
struct GTY(()) tree_node {
    int GTY((skip)) value;
    struct tree_node *GTY((skip)) left;
    struct tree_node *GTY((skip)) right;
    struct tree_node *GTY((skip)) parent;
};

/* Conditional compilation with GTY */
#ifdef USE_GTY_MARKERS
struct GTY(()) conditional_struct {
    int GTY((skip)) x;
    char GTY((skip)) y;
};
#else
struct conditional_struct {
    int x;
    char y;
};
#endif

/* Multiple GTY options */
struct GTY(()) options_test {
    int GTY((skip, tag("0"))) tagged_field;
    struct tree_node *GTY((skip, length("len"))) node_array;
    int GTY((skip)) len;
    const_string GTY((skip)) description;
};

/* Simulating lang_struct-like behavior */
struct GTY((tag("LANG_STRUCT"))) lang_simul {
    int GTY((skip)) lang_specific;
    void *GTY((skip)) lang_data;
};

/* Forward declarations with GTY */
struct GTY(()) forward_declared;
typedef struct forward_declared GTY(()) forward_declared_t;

#ifdef __cplusplus
}
#endif

#endif /* GTY_TEST_TYPES_H */

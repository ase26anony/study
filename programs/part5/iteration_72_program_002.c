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
typedef int GTY(()) int_matrix[5][5];

/* User-defined struct type - TYPE_USER_STRUCT */
typedef struct base_struct GTY(()) base_struct_t;
typedef union data_union GTY(()) data_union_t;

/* Callback type - TYPE_CALLBACK */
typedef void (*GTY(()) callback_func)(int, void*);
typedef int (*GTY(()) compare_func)(const void*, const void*);

/* String type - TYPE_STRING */
typedef const char *GTY(()) const_string;

/* Scalar type in container - TYPE_SCALAR */
struct GTY(()) scalar_container {
    long GTY((skip)) scalar1;
    unsigned long long GTY((skip)) scalar2;
    double GTY((skip)) scalar3;
};

/* Complex nested structure */
struct GTY(()) complex_nested {
    struct list_node *GTY((skip)) head;
    struct list_node *GTY((skip)) tail;
    union data_union GTY((skip)) current;
    struct_array GTY((skip)) backup;
    callback_func GTY((skip)) handler;
};

/* Recursive type structure */
struct GTY(()) tree_node {
    int GTY((skip)) value;
    struct tree_node *GTY((skip)) left;
    struct tree_node *GTY((skip)) right;
    struct tree_node *GTY((skip)) parent;
};

/* Union containing structure, structure containing union */
struct GTY(()) struct_with_union {
    int GTY((skip)) type;
    union {
        int GTY((skip)) int_member;
        float GTY((skip)) float_member;
        struct base_struct *GTY((skip)) struct_member;
    } GTY((skip)) data;
};

/* Array of pointers to different GTY-marked types */
typedef struct base_struct *GTY(()) base_ptr_array[20];
typedef union data_union *GTY(()) union_ptr_array[15];
typedef struct tree_node *GTY(()) tree_ptr_array[30];

/* Conditional compilation with GTY */
#ifdef USE_GTY_OPTIONS
struct GTY((skip, tag("special_tag"))) tagged_struct {
    int GTY((skip)) special_field;
    void *GTY((length("len"))) variable_array;
    int len;
};
#else
struct GTY(()) tagged_struct {
    int GTY((skip)) regular_field;
    void *GTY((skip)) data;
};
#endif

/* Simulating lang_struct pattern - TYPE_LANG_STRUCT */
/* In GCC, lang_structs are often marked with special patterns */
struct GTY((chain_next ("%h.next"), chain_prev ("%h.prev"))) lang_struct_sim {
    struct lang_struct_sim *GTY((skip)) next;
    struct lang_struct_sim *GTY((skip)) prev;
    int GTY((skip)) lang_specific;
};

/* Multiple GTY options chained */
struct GTY((skip, reorder, for_user)) multi_option_struct {
    int GTY((skip)) a;
    char GTY((skip)) b;
    long GTY((skip)) c;
};

/* Function pointer with complex signature */
typedef struct base_struct* (*GTY(()) factory_func)(int, const char*);
typedef void (*GTY(()) destructor_func)(void*);

#endif /* GTY_TEST_TYPES_H */

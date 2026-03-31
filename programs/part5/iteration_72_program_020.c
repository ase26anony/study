#ifndef GTY_TYPES_H
#define GTY_TYPES_H

/* Basic GTY-marked structure */
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

/* Recursive structure */
struct GTY(()) tree_node {
    int value;
    struct tree_node *GTY((skip)) left;
    struct tree_node *GTY((skip)) right;
    union data_union GTY((skip)) data;
};

/* Array type definition */
typedef int GTY(()) int_array[10];
typedef struct base_struct *GTY(()) struct_ptr_array[5];

/* Callback (function pointer) type */
typedef void (*GTY(()) callback_func)(int, struct base_struct*);

/* Structure containing callback */
struct GTY(()) callback_container {
    callback_func GTY((skip)) handler;
    void *GTY((skip)) user_data;
};

/* Nested structure with union */
struct GTY(()) complex_type {
    struct base_struct base;
    union {
        int counter;
        float ratio;
    } GTY((skip)) metrics;
    struct_ptr_array refs;
};

/* Scalar type in container */
struct GTY(()) scalar_box {
    long GTY((skip)) value;
    unsigned GTY((skip)) flags;
};

/* Conditional compilation with GTY */
#ifdef USE_SPECIAL_TYPES
struct GTY(()) special_struct {
    int magic;
    struct complex_type *GTY((skip)) link;
};
#else
struct special_struct {
    int magic;
    void *link;
};
#endif

/* User-defined struct type via typedef */
typedef struct base_struct GTY(()) base_struct_t;
typedef union data_union GTY(()) data_union_t;

/* Language-like structure (simulating TYPE_LANG_STRUCT) */
struct GTY(()) lang_struct {
    int lang_specific;
    struct GTY((tag("tree_node"))) tree_node *ast;  /* Using tag option */
};

/* Self-referential union */
union GTY(()) recursive_union {
    int terminal;
    struct GTY((skip)) recursive_union *next;
};

#endif /* GTY_TYPES_H */

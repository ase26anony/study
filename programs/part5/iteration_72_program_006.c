#ifndef GTY_TYPES_H
#define GTY_TYPES_H

/* Basic GTY-marked structure - TYPE_STRUCT */
struct GTY(()) base_struct {
    int id;
    char *GTY((skip)) name;  /* TYPE_POINTER with skip option */
};

/* User-defined structure type via typedef - TYPE_USER_STRUCT */
typedef struct base_struct GTY(()) base_struct_t;

/* Union type - TYPE_UNION */
union GTY(()) data_union {
    int int_val;
    float float_val;
    void *GTY((skip)) ptr_val;  /* TYPE_POINTER */
    struct base_struct *GTY((skip)) struct_ptr;  /* Nested pointer */
};

/* Array type - TYPE_ARRAY */
typedef int GTY(()) int_array[10];

/* Multi-dimensional array */
typedef struct base_struct *GTY(()) struct_ptr_array[5][3];

/* Callback (function pointer) type - TYPE_CALLBACK */
typedef void (*GTY(()) callback_func)(int, const char*);

/* String type - TYPE_STRING */
typedef const char *GTY(()) gty_string;

/* Scalar type in container - TYPE_SCALAR */
struct GTY(()) scalar_container {
    long GTY((skip)) scalar_value;
    unsigned GTY((skip)) flags;
};

/* Recursive structure for complex type graphs */
struct GTY(()) tree_node {
    int value;
    struct tree_node *GTY((skip)) left;   /* Recursive pointer */
    struct tree_node *GTY((skip)) right;  /* Recursive pointer */
    union data_union GTY((skip)) data;    /* Nested union */
};

/* Structure containing various GTY-marked members */
struct GTY(()) complex_struct {
    base_struct_t base;                    /* TYPE_USER_STRUCT */
    union data_union variant;              /* TYPE_UNION */
    int_array numbers;                     /* TYPE_ARRAY */
    struct_ptr_array ptr_matrix;           /* TYPE_ARRAY of TYPE_POINTER */
    callback_func callback;                /* TYPE_CALLBACK */
    gty_string message;                    /* TYPE_STRING */
    struct complex_struct *GTY((skip)) next; /* Recursive pointer */
};

/* Conditional compilation with GTY */
#ifdef USE_SPECIAL_TYPES
struct GTY(()) special_struct {
    int special_id;
    void *GTY((special)) special_data;
};
#else
struct special_struct {
    int special_id;
    void *special_data;
};
#endif

/* GTY with multiple options */
struct GTY(()) optioned_struct {
    int GTY((tag("1"), skip)) tagged_field;
    struct tree_node *GTY((chain_next, chain_prev)) chain_node;
    int GTY((skip)) ignored_field;
};

/* Simulating lang_struct pattern - TYPE_LANG_STRUCT */
/* In GCC, lang_struct types often have specific naming patterns */
struct GTY(()) lang_type {
    int lang_specific;
    void *GTY((skip)) lang_data;
};

/* Forward declaration for mutual recursion */
struct GTY(()) forward_decl_struct;
struct GTY(()) container_struct;

/* Complete the forward declaration */
struct GTY(()) forward_decl_struct {
    int id;
    struct container_struct *GTY((skip)) container;
};

struct GTY(()) container_struct {
    int container_id;
    struct forward_decl_struct *GTY((skip)) item;
    struct forward_decl_struct GTY((skip)) items[5];  /* Array of structs */
};

/* Enumeration type (treated as scalar) */
typedef enum GTY(()) color {
    RED,
    GREEN,
    BLUE
} color_t;

/* Structure with enum */
struct GTY(()) colored_object {
    color_t color;
    int size;
    struct colored_object *GTY((skip)) next;
};

#endif /* GTY_TYPES_H */

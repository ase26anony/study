#ifndef GTY_TYPES_H
#define GTY_TYPES_H

/* Basic GTY-marked structure - TYPE_STRUCT */
struct GTY(()) base_struct {
    int id;
    char *GTY((skip)) name;  /* TYPE_POINTER with skip option */
};

/* User-defined structure type - TYPE_USER_STRUCT */
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
typedef struct base_struct *GTY(()) struct_ptr_array[5];

/* Pointer type - TYPE_POINTER */
typedef struct base_struct *GTY(()) base_struct_ptr;

/* Callback type - TYPE_CALLBACK */
typedef void (*GTY(()) callback_func)(int, void*);

/* Scalar type in container - TYPE_SCALAR */
struct GTY(()) scalar_container {
    long GTY((skip)) scalar_value;
    unsigned GTY((skip)) flags;
};

/* String type - TYPE_STRING */
struct GTY(()) string_container {
    const char *GTY((skip)) message;  /* String pointer */
    char *GTY((skip)) dynamic_str;
};

/* Recursive structure for complex type graphs */
struct GTY(()) tree_node {
    int value;
    struct tree_node *GTY((skip)) left;   /* Recursive pointer */
    struct tree_node *GTY((skip)) right;  /* Recursive pointer */
    union data_union GTY((skip)) data;    /* Nested union */
};

/* Structure containing array */
struct GTY(()) array_container {
    int GTY((skip)) numbers[20];  /* Array field */
    struct base_struct GTY((skip)) items[5];  /* Array of structs */
};

/* Conditional compilation with GTY */
#ifdef USE_GTY_MARKERS
struct GTY(()) conditional_struct {
    int x;
    double y;
};
#else
struct conditional_struct {
    int x;
    double y;
};
#endif

/* Multiple GTY options chained */
struct GTY(()) complex_options {
    int *GTY((skip, tag("special_ptr"))) special;
    struct tree_node *GTY((skip)) nodes[10];
};

/* Forward declaration for mutual recursion */
struct GTY(()) list_node;

/* Complete definition with mutual recursion */
struct GTY(()) list_node {
    int data;
    struct list_node *GTY((skip)) next;
    struct tree_node *GTY((skip)) tree_ref;
};

/* Simulating lang_struct pattern - TYPE_LANG_STRUCT */
/* In GCC, lang_structs are often marked with special tags */
struct GTY((tag("LANG_STRUCT"))) lang_simulated {
    int lang_specific;
    void *GTY((skip)) lang_data;
};

/* Function pointer with complex signature */
typedef int (*GTY(()) complex_callback)(
    struct base_struct *GTY((skip)),
    union data_union *GTY((skip)),
    callback_func GTY((skip))
);

#endif /* GTY_TYPES_H */

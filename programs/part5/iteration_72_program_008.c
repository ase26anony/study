#ifndef GTY_TYPES_H
#define GTY_TYPES_H

/* Basic GTY-marked structure - TYPE_STRUCT */
struct GTY(()) base_struct {
    int id;
    char *GTY((skip)) name;  /* TYPE_POINTER with skip option */
};

/* GTY-marked union - TYPE_UNION */
union GTY(()) data_union {
    int int_val;
    float float_val;
    void *GTY((skip)) ptr_val;  /* TYPE_POINTER */
    struct base_struct *GTY((skip)) struct_ptr;  /* Nested pointer */
};

/* Typedef creating user-defined type - TYPE_USER_STRUCT */
typedef struct base_struct GTY(()) base_struct_t;

/* Array type - TYPE_ARRAY */
typedef int GTY(()) int_array[10];
typedef struct base_struct *GTY(()) struct_ptr_array[5];

/* Function pointer (callback) type - TYPE_CALLBACK */
typedef void (*GTY(()) callback_func)(int, const char*);
typedef int (*GTY(()) compare_func)(const void*, const void*);

/* Recursive structure with pointer to self */
struct GTY(()) tree_node {
    int value;
    struct tree_node *GTY((skip)) left;
    struct tree_node *GTY((skip)) right;
    union data_union GTY((skip)) data;  /* Union inside struct */
};

/* Structure containing array of pointers */
struct GTY(()) container {
    int count;
    struct base_struct *GTY((skip)) items[20];  /* Array of pointers */
    int_array numbers;  /* Array of scalars */
};

/* Scalar type in container - TYPE_SCALAR */
struct GTY(()) scalar_container {
    long GTY((skip)) scalar1;
    unsigned long long GTY((skip)) scalar2;
    double GTY((skip)) scalar3;
};

/* String type - TYPE_STRING */
struct GTY(()) string_holder {
    const char *GTY((skip)) str;  /* String pointer */
    char *GTY((skip)) mutable_str;
};

/* Complex nested type */
struct GTY(()) complex_type {
    struct container GTY((skip)) cont;
    union data_union GTY((skip)) uni;
    callback_func GTY((skip)) callback;
    struct complex_type *GTY((skip)) next;  /* Linked list */
};

/* Conditional compilation with GTY */
#ifdef USE_GTY_MARKERS
struct GTY(()) conditional_struct {
    int x;
    char *GTY((skip)) y;
};
#else
struct conditional_struct {
    int x;
    char *y;
};
#endif

/* Multiple GTY options chained */
struct GTY(()) options_test {
    int GTY((skip, tag("special_field"))) special;
    void *GTY((skip, length("len"))) variable_array;
    int len;
};

/* Forward declaration for mutual recursion */
struct GTY(()) forward_decl;
struct GTY(()) another_struct;

/* Mutual recursion */
struct GTY(()) forward_decl {
    int id;
    struct another_struct *GTY((skip)) partner;
};

struct GTY(()) another_struct {
    int id;
    struct forward_decl *GTY((skip)) partner;
};

/* Enumeration (not GTY-marked, for contrast) */
enum color { RED, GREEN, BLUE };

/* GTY-marked structure with enum */
struct GTY(()) enum_container {
    enum color GTY((skip)) color;
    int value;
};

/* Simulating lang_struct pattern - TYPE_LANG_STRUCT */
/* In GCC, lang_structs are often tree-related types */
struct GTY((tag("tree_common"))) tree_common {
    int code;
    union data_union GTY((skip)) u;
};

/* Another lang_struct-like pattern */
struct GTY((tag("gimple_statement"))) gimple_stmt {
    int type;
    struct tree_common *GTY((skip)) expr;
};

/* Type with callback field */
struct GTY(()) callback_container {
    callback_func GTY((skip)) handler;
    compare_func GTY((skip)) comparator;
    void *GTY((skip)) user_data;
};

#endif /* GTY_TYPES_H */

#ifndef GTY_TEST_TYPES_H
#define GTY_TEST_TYPES_H

/* Basic structure type - TYPE_STRUCT */
struct GTY(()) base_struct {
    int id;
    char *GTY((skip)) name;  /* Skip this field */
};

/* Union type - TYPE_UNION */
union GTY(()) data_union {
    int int_val;
    float float_val;
    void *GTY((skip)) ptr_val;
    struct base_struct *GTY((tag("0"))) struct_ptr;
};

/* User-defined struct type via typedef - TYPE_USER_STRUCT */
typedef struct base_struct GTY(()) base_struct_t;

/* Pointer type within structure - TYPE_POINTER */
struct GTY(()) linked_node {
    struct linked_node *GTY((skip)) next;
    struct linked_node *GTY((skip)) prev;
    union data_union data;
    int GTY((length("data_length"))) *dynamic_array;  /* Array with length */
    unsigned data_length;
};

/* Array type definition - TYPE_ARRAY */
typedef struct base_struct GTY(()) struct_array[10];

/* Function pointer (callback) type - TYPE_CALLBACK */
typedef void (*GTY(()) callback_func)(int, const char*);

/* Structure containing callback */
struct GTY(()) callback_container {
    callback_func GTY((skip)) handler;
    int callback_id;
};

/* String type handling - TYPE_STRING */
struct GTY(()) string_holder {
    const char *GTY((skip)) constant_string;
    char *GTY((skip)) mutable_string;
};

/* Scalar type in container - TYPE_SCALAR */
struct GTY(()) scalar_box {
    long GTY((skip)) long_value;
    double GTY((skip)) double_value;
    _Complex double GTY((skip)) complex_value;
};

/* Recursive type structure */
struct GTY(()) tree_node {
    int value;
    struct tree_node *GTY((skip)) left;
    struct tree_node *GTY((skip)) right;
    struct tree_node **GTY((skip)) children;  /* Array of pointers */
    unsigned child_count;
};

/* Union containing structure, structure containing union */
struct GTY(()) union_container {
    union data_union u;
    int type_tag;
};

union GTY(()) struct_container {
    struct base_struct s;
    struct linked_node l;
    double padding[4];
};

/* Forward declarations for mutual recursion */
struct GTY(()) forward_decl_a;
struct GTY(()) forward_decl_b;

struct GTY(()) forward_decl_a {
    int a_val;
    struct forward_decl_b *GTY((skip)) b_ptr;
};

struct GTY(()) forward_decl_b {
    int b_val;
    struct forward_decl_a *GTY((skip)) a_ptr;
};

/* Conditional compilation with GTY */
#ifdef USE_GTY_OPTIONS
struct GTY((chain_next("next"), chain_prev("prev"))) chained_struct {
    int data;
    struct chained_struct *next;
    struct chained_struct *prev;
};
#else
struct chained_struct {
    int data;
    struct chained_struct *next;
    struct chained_struct *prev;
};
#endif

/* Array of pointers to different GTY-marked types */
typedef struct GTY(()) variant_ptr {
    enum { PTR_TO_STRUCT, PTR_TO_UNION, PTR_TO_ARRAY } ptr_type;
    union {
        struct base_struct *GTY((skip)) s_ptr;
        union data_union *GTY((skip)) u_ptr;
        struct_array *GTY((skip)) a_ptr;
    } ptr;
} variant_ptr_t;

/* Simulating lang_struct pattern - TYPE_LANG_STRUCT */
/* In GCC, lang_struct types often have special handling */
struct GTY(()) lang_simul {
    struct GTY((tag("1"))) base_struct base;
    void *GTY((skip)) lang_specific;
};

#endif /* GTY_TEST_TYPES_H */

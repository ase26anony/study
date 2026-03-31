/* Test header for gengtype coverage - targeting TYPE_* switch cases */
#ifndef GTYPE_TEST_H
#define GTYPE_TEST_H

#include "gtype-desc.h"

/* ==================== TYPE_SCALAR ==================== */
typedef int scalar_int_t;
typedef double scalar_double_t;
typedef enum { RED, GREEN, BLUE } color_enum_t;

/* ==================== TYPE_STRING ==================== */
struct GTY(()) gcc_string {
    int length;
    char *data;
};

/* ==================== TYPE_CALLBACK ==================== */
typedef int (*callback_t)(void*);
typedef void (*void_callback_t)(int, double);

/* ==================== TYPE_UNDEFINED (non-GTY types) ==================== */
/* These types are not marked with GTY but will be referenced by GTY types */
struct non_gty_struct {
    int x;
    float y;
};

typedef struct non_gty_struct* non_gty_ptr_t;

/* ==================== TYPE_STRUCT ==================== */
struct GTY(()) base_struct {
    int id;
    scalar_double_t value;
    color_enum_t color;
};

struct GTY(()) complex_struct {
    struct base_struct *base;          /* TYPE_POINTER to TYPE_STRUCT */
    struct gcc_string *name;           /* TYPE_POINTER to TYPE_STRING */
    callback_t handler;                /* TYPE_CALLBACK */
    int flags[8];                      /* TYPE_ARRAY of TYPE_SCALAR */
};

/* ==================== TYPE_USER_STRUCT ==================== */
struct GTY((user)) user_defined_struct {
    int user_id;
    char *user_name;
    void *user_data;  /* Opaque pointer */
};

/* ==================== TYPE_UNION ==================== */
union GTY(()) data_union {
    int as_int;
    double as_double;
    char *as_string;
    struct base_struct *as_struct;
};

/* ==================== TYPE_ARRAY ==================== */
typedef int vec4_t[4];
typedef struct base_struct* struct_ptr_array_t[10];

struct GTY(()) array_container {
    vec4_t vector;                     /* TYPE_ARRAY of TYPE_SCALAR */
    struct_ptr_array_t pointers;       /* TYPE_ARRAY of TYPE_POINTER */
    int matrix[3][3];                  /* Multi-dimensional TYPE_ARRAY */
};

/* ==================== TYPE_POINTER ==================== */
typedef struct complex_struct* complex_ptr_t;
typedef union data_union* union_ptr_t;

/* Linked list structure for deep traversal */
struct GTY(()) linked_node {
    int data;
    struct linked_node *next;          /* TYPE_POINTER to TYPE_STRUCT */
    struct linked_node *prev;          /* TYPE_POINTER to TYPE_STRUCT */
};

/* Tree structure for more complex graphs */
struct GTY(()) tree_node {
    int value;
    struct tree_node *left;            /* TYPE_POINTER */
    struct tree_node *right;           /* TYPE_POINTER */
    struct tree_node *parent;          /* TYPE_POINTER */
};

/* ==================== TYPE_LANG_STRUCT ==================== */
/* This typically requires special handling in GCC frontends */
/* We'll simulate it with a forward declaration and special marker */
struct GTY((desc("%0"))) lang_specific {
    int lang_tag;
    void *lang_data;
};

/* ==================== ROOT STRUCTURE ==================== */
/* This contains pointers to all other types to ensure full traversal */
struct GTY(()) root_container {
    struct base_struct *base;          /* TYPE_POINTER to TYPE_STRUCT */
    struct complex_struct *complex;    /* TYPE_POINTER to TYPE_STRUCT */
    struct user_defined_struct *user;  /* TYPE_POINTER to TYPE_USER_STRUCT */
    union data_union data;             /* TYPE_UNION */
    struct array_container arrays;     /* TYPE_STRUCT with TYPE_ARRAY members */
    struct linked_node *list_head;     /* TYPE_POINTER to linked list */
    struct tree_node *tree_root;       /* TYPE_POINTER to tree */
    struct gcc_string *description;    /* TYPE_POINTER to TYPE_STRING */
    callback_t callbacks[5];           /* TYPE_ARRAY of TYPE_CALLBACK */
    struct lang_specific *lang;        /* TYPE_POINTER to TYPE_LANG_STRUCT */
    
    /* References to non-GTY types (may trigger TYPE_UNDEFINED) */
    struct non_gty_struct *non_gty_ref; /* TYPE_POINTER to non-GTY type */
    non_gty_ptr_t non_gty_ptr;          /* TYPE_POINTER via typedef */
    
    /* Direct scalar members */
    scalar_int_t counter;              /* TYPE_SCALAR */
    scalar_double_t total;             /* TYPE_SCALAR */
    color_enum_t theme;                /* TYPE_SCALAR (enum) */
};

/* Function pointer table */
typedef struct GTY(()) {
    const char *name;
    void_callback_t func;
} func_table_entry_t;

/* Array of structs containing function pointers */
struct GTY(()) func_registry {
    int count;
    func_table_entry_t entries[20];    /* TYPE_ARRAY of TYPE_STRUCT */
};

/* ==================== Additional complex types ==================== */
/* Struct with nested anonymous struct */
struct GTY(()) outer_struct {
    int outer_id;
    struct GTY(()) {
        int inner_x;
        double inner_y;
        struct outer_struct *parent;   /* TYPE_POINTER back to parent */
    } inner;
};

/* Union within struct */
struct GTY(()) variant_container {
    int type;
    union GTY(()) {
        int int_val;
        double double_val;
        char *string_val;
        struct base_struct *struct_val;
    } data;
};

#endif /* GTYPE_TEST_H */

/* Test header for gengtype-state.cc coverage */
#ifndef GTY_TEST_H
#define GTY_TEST_H

#include "gtype-desc.h"

/* TYPE_SCALAR: Basic scalar types */
typedef int scalar_int_t;
typedef double scalar_double_t;

/* Enum type (also scalar) */
enum color { RED, GREEN, BLUE };

/* TYPE_ARRAY: Array typedef */
typedef int vec4_t[4];
typedef char name_array_t[32];

/* TYPE_CALLBACK: Function pointer type */
typedef int (*callback_t)(void *data, int param);
typedef void (*simple_cb_t)(void);

/* Non-GTY struct (may become TYPE_UNDEFINED when referenced) */
struct non_gty_struct {
    int hidden_data;
    char *hidden_string;
};

/* TYPE_STRUCT: Basic struct with GTY marker */
struct GTY(()) basic_struct {
    int id;                     /* TYPE_SCALAR */
    char *name;                 /* TYPE_STRING */
    double value;               /* TYPE_SCALAR */
};

/* TYPE_STRUCT with nested pointer */
struct GTY(()) container_struct {
    struct basic_struct *item;  /* TYPE_POINTER to TYPE_STRUCT */
    int count;                  /* TYPE_SCALAR */
    vec4_t coordinates;         /* TYPE_ARRAY */
};

/* TYPE_UNION: Union type */
union GTY(()) data_union {
    int as_int;                 /* TYPE_SCALAR */
    double as_double;           /* TYPE_SCALAR */
    char *as_string;            /* TYPE_STRING */
    void *as_pointer;           /* TYPE_POINTER */
};

/* TYPE_STRUCT with union member */
struct GTY(()) struct_with_union {
    int type_tag;               /* TYPE_SCALAR */
    union data_union data;      /* TYPE_UNION */
    callback_t handler;         /* TYPE_CALLBACK */
};

/* TYPE_POINTER: Various pointer types in a struct */
struct GTY(()) pointer_collection {
    int *int_ptr;               /* TYPE_POINTER to TYPE_SCALAR */
    char **string_pp;           /* TYPE_POINTER to TYPE_POINTER to TYPE_STRING */
    struct basic_struct *struct_ptr;  /* TYPE_POINTER to TYPE_STRUCT */
    union data_union *union_ptr;      /* TYPE_POINTER to TYPE_UNION */
    callback_t *callback_ptr;   /* TYPE_POINTER to TYPE_CALLBACK */
    struct non_gty_struct *hidden_ptr; /* TYPE_POINTER to potentially TYPE_UNDEFINED */
};

/* TYPE_ARRAY of pointers */
struct GTY(()) array_of_pointers {
    struct basic_struct *items[10];     /* TYPE_ARRAY of TYPE_POINTER */
    callback_t handlers[5];             /* TYPE_ARRAY of TYPE_CALLBACK */
    int *int_ptrs[8];                   /* TYPE_ARRAY of TYPE_POINTER */
};

/* TYPE_STRING: String-like structure */
struct GTY(()) gcc_string {
    int length;                 /* TYPE_SCALAR */
    char *data;                 /* TYPE_STRING */
    const char *const_data;     /* TYPE_STRING (const variant) */
};

/* Linked list structure (recursive pointer) */
struct GTY(()) linked_node {
    int data;                   /* TYPE_SCALAR */
    struct linked_node *next;   /* TYPE_POINTER (recursive) */
    struct linked_node *prev;   /* TYPE_POINTER (recursive) */
};

/* TYPE_USER_STRUCT: User-defined struct with special handling */
struct GTY((user)) user_defined_struct {
    void *opaque_data;          /* TYPE_POINTER */
    int user_id;                /* TYPE_SCALAR */
};

/* Complex nested structure */
struct GTY(()) complex_nested {
    struct container_struct container;  /* TYPE_STRUCT */
    union data_union variants[3];       /* TYPE_ARRAY of TYPE_UNION */
    struct gcc_string str;              /* TYPE_STRUCT (string-like) */
    struct pointer_collection *ptr_collection; /* TYPE_POINTER */
};

/* TYPE_LANG_STRUCT: Language-specific structure */
struct GTY((desc("%1"), tag("lang_type"))) lang_struct {
    int lang_specific_flag;     /* TYPE_SCALAR */
    void *lang_data;            /* TYPE_POINTER */
    struct GTY((skip)) skipped_part; /* Part to skip in GC */
};

/* Root structure containing pointers to everything */
struct GTY(()) root_container {
    struct basic_struct *basic;         /* TYPE_POINTER */
    struct container_struct *container; /* TYPE_POINTER */
    struct struct_with_union *with_union; /* TYPE_POINTER */
    struct pointer_collection *pointers; /* TYPE_POINTER */
    struct array_of_pointers *arrays;   /* TYPE_POINTER */
    struct gcc_string *string_obj;      /* TYPE_POINTER */
    struct linked_node *list_head;      /* TYPE_POINTER */
    struct user_defined_struct *user_struct; /* TYPE_POINTER */
    struct complex_nested *nested;      /* TYPE_POINTER */
    struct lang_struct *lang;           /* TYPE_POINTER */
    
    /* Direct members */
    scalar_int_t direct_int;            /* TYPE_SCALAR */
    callback_t direct_callback;         /* TYPE_CALLBACK */
    vec4_t direct_array;                /* TYPE_ARRAY */
    union data_union direct_union;      /* TYPE_UNION */
};

/* Additional pointer typedefs */
typedef struct basic_struct *basic_ptr_t;
typedef union data_union *union_ptr_t;
typedef callback_t *callback_ptr_t;

/* Array of structures */
typedef struct basic_struct basic_array_t[5];
typedef union data_union union_array_t[3];

#endif /* GTY_TEST_H */

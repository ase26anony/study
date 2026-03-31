/* gty-test.h - Test file for gengtype type classification coverage */

#ifndef GTY_TEST_H
#define GTY_TEST_H

/* Forward declaration for TYPE_UNDEFINED case */
struct opaque;

/* TYPE_SCALAR: plain scalar type */
typedef int GTY(()) scalar_type;

/* TYPE_STRING: string type */
typedef char *GTY(()) string_type;

/* TYPE_STRUCT: basic struct */
struct GTY(()) basic_struct {
    int scalar_field;          /* TYPE_SCALAR */
    char *string_field;        /* TYPE_STRING */
    struct opaque *opaque_ptr; /* May influence TYPE_UNDEFINED */
};

/* TYPE_USER_STRUCT: typedef'd struct */
typedef struct GTY(()) {
    int data;
    struct basic_struct *next; /* TYPE_POINTER */
} user_struct_type;

/* TYPE_UNION: union type */
union GTY(()) test_union {
    int int_val;
    char *str_val;            /* TYPE_STRING */
    struct basic_struct *sptr; /* TYPE_POINTER */
};

/* TYPE_POINTER: pointer type in struct */
struct GTY(()) pointer_container {
    struct basic_struct *struct_ptr;   /* TYPE_POINTER to TYPE_STRUCT */
    user_struct_type *user_struct_ptr; /* TYPE_POINTER to TYPE_USER_STRUCT */
    union test_union *union_ptr;       /* TYPE_POINTER to TYPE_UNION */
};

/* TYPE_ARRAY: array types */
struct GTY(()) array_container {
    int scalar_array[10];               /* TYPE_ARRAY of TYPE_SCALAR */
    struct basic_struct *ptr_array[5];  /* TYPE_ARRAY of TYPE_POINTER */
    char *string_array[3];              /* TYPE_ARRAY of TYPE_STRING */
};

/* TYPE_CALLBACK: function pointer */
typedef void (*GTY(()) callback_type)(int, char *);

struct GTY(()) callback_container {
    callback_type handler;              /* TYPE_CALLBACK */
    void (*GTY(()) direct_callback)(void); /* TYPE_CALLBACK directly */
};

/* TYPE_LANG_STRUCT: Typically used for language-specific structures */
/* In GCC, these are marked with GTY(()) and special lang_struct tags */
/* We'll simulate this with a struct that might be treated specially */
struct GTY(()) lang_struct_sim {
    int lang_specific_data;
    struct GTY(()) nested_lang *next; /* Self-referential for testing */
};

/* Nested structures for comprehensive testing */
struct GTY(()) outer_struct {
    struct GTY(()) {
        int inner_data;
        char *inner_string;
    } inner;
    
    union GTY(()) {
        int option_a;
        struct inner *option_b;
    } choice;
};

/* Multiple levels of indirection */
typedef struct GTY(()) node {
    int value;
    struct node **GTY((skip)) double_ptr; /* Skip this for pointer counting */
    struct node *next;                    /* Regular pointer */
} node_t;

/* Template-like structure (common in GCC) */
struct GTY(()) template_struct {
    union GTY(()) {
        int int_template;
        double double_template;
    } GTY((desc("%0.template_kind"))) template_data;
    
    int template_kind;
};

#endif /* GTY_TEST_H */

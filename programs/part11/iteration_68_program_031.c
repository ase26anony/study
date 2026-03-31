/* gty-test.h - Test file for gengtype type classification coverage */

#ifndef GTY_TEST_H
#define GTY_TEST_H

/* Forward declaration for TYPE_UNDEFINED case */
struct opaque;

/* TYPE_SCALAR: plain scalar type */
typedef int GTY(()) scalar_type;

/* TYPE_STRING: string type */
typedef char *GTY(()) string_type;

/* TYPE_STRUCT: regular struct */
struct GTY(()) my_struct {
    int GTY(()) scalar_field;      /* TYPE_SCALAR */
    char *GTY(()) string_field;    /* TYPE_STRING */
    struct opaque *GTY(()) opaque_ptr; /* May trigger TYPE_UNDEFINED */
};

/* TYPE_USER_STRUCT: typedef'd struct */
typedef struct GTY(()) {
    int GTY(()) data;
    struct my_struct *GTY(()) next; /* TYPE_POINTER */
} user_struct_type;

/* TYPE_UNION: union type */
union GTY(()) my_union {
    int GTY(()) int_val;
    char *GTY(()) str_val;
    struct my_struct GTY(()) struct_val;
};

/* TYPE_POINTER: pointer type in struct */
struct GTY(()) pointer_container {
    struct my_struct *GTY(()) struct_ptr;  /* TYPE_POINTER */
    union my_union *GTY(()) union_ptr;     /* TYPE_POINTER */
    int *GTY(()) int_ptr;                  /* TYPE_POINTER */
};

/* TYPE_ARRAY: array type */
struct GTY(()) array_container {
    int GTY(()) int_array[10];             /* TYPE_ARRAY */
    struct my_struct GTY(()) struct_array[5]; /* TYPE_ARRAY */
    char *GTY(()) string_array[3];         /* TYPE_ARRAY of TYPE_STRING */
};

/* TYPE_CALLBACK: function pointer */
typedef void (*GTY(()) callback_type)(int, char *);

struct GTY(()) callback_container {
    callback_type GTY(()) handler;         /* TYPE_CALLBACK */
    void (*GTY(()) another_handler)(void); /* TYPE_CALLBACK */
};

/* TYPE_LANG_STRUCT: Language-specific structure */
/* This typically requires special handling in gengtype */
/* We'll use a struct with a tag that might be recognized as language-specific */
struct GTY(()) lang_struct {
    int GTY(()) lang_specific_data;
    /* In real GCC, this would have fields specific to a language frontend */
};

/* Complex nested structure to ensure all counters are hit */
struct GTY(()) complex_type {
    /* TYPE_SCALAR */
    int GTY(()) count;
    
    /* TYPE_STRING */
    char *GTY(()) name;
    
    /* TYPE_POINTER */
    struct complex_type *GTY(()) next;
    
    /* TYPE_ARRAY */
    callback_type GTY(()) callbacks[5];
    
    /* TYPE_STRUCT (embedded) */
    struct my_struct GTY(()) embedded;
    
    /* TYPE_UNION (embedded) */
    union my_union GTY(()) choice;
    
    /* TYPE_CALLBACK */
    callback_type GTY(()) single_callback;
    
    /* Pointer to undefined type */
    struct undefined_type *GTY(()) undefined_ptr;
};

#endif /* GTY_TEST_H */

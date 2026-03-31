/* gty-test.h - Test file for gengtype type classification coverage */

#ifndef GTY_TEST_H
#define GTY_TEST_H

/* Forward declaration for TYPE_UNDEFINED test */
struct opaque;

/* TYPE_SCALAR: plain scalar type */
typedef int GTY(()) scalar_type;

/* TYPE_STRING: string type */
typedef const char * GTY(()) string_type;

/* TYPE_STRUCT: basic struct */
struct GTY(()) basic_struct {
    int GTY(()) scalar_field;      /* TYPE_SCALAR */
    char * GTY(()) string_field;   /* TYPE_STRING */
};

/* TYPE_USER_STRUCT: typedef'd struct */
typedef struct GTY(()) {
    int x;
    int y;
} user_struct_type;

/* TYPE_UNION: union type */
union GTY(()) test_union {
    int GTY(()) int_val;
    char * GTY(()) str_val;
    struct basic_struct * GTY(()) struct_ptr;
};

/* TYPE_POINTER: pointer type within struct */
struct GTY(()) pointer_container {
    struct basic_struct * GTY(()) ptr_field;      /* TYPE_POINTER */
    struct opaque * GTY(()) opaque_ptr;           /* TYPE_UNDEFINED via pointer */
};

/* TYPE_ARRAY: array type */
struct GTY(()) array_container {
    int GTY(()) scalar_array[10];                 /* TYPE_ARRAY of TYPE_SCALAR */
    struct basic_struct * GTY(()) ptr_array[5];   /* TYPE_ARRAY of TYPE_POINTER */
};

/* TYPE_CALLBACK: function pointer */
typedef void (* GTY(()) callback_type)(int, const char*);

struct GTY(()) callback_container {
    callback_type GTY(()) cb;                     /* TYPE_CALLBACK */
    void (* GTY(()) direct_cb)(struct basic_struct*); /* Another callback */
};

/* TYPE_LANG_STRUCT: language-specific struct */
/* This requires special handling - typically marked with GTY((tag("..."))) */
struct GTY((tag("LANG_STRUCT"))) lang_specific {
    int lang_specific_field;
};

/* Complex nested structure to ensure all counters are hit */
struct GTY(()) master_container {
    /* Direct fields */
    int GTY(()) count;                           /* TYPE_SCALAR */
    char * GTY(()) name;                         /* TYPE_STRING */
    
    /* Nested structures */
    struct basic_struct GTY(()) nested_struct;   /* TYPE_STRUCT */
    union test_union GTY(()) nested_union;       /* TYPE_UNION */
    
    /* Pointers */
    struct pointer_container * GTY(()) ptr;      /* TYPE_POINTER */
    
    /* Arrays */
    int GTY(()) numbers[20];                     /* TYPE_ARRAY */
    
    /* Callback */
    callback_type GTY(()) handler;               /* TYPE_CALLBACK */
    
    /* Language-specific */
    struct lang_specific * GTY(()) lang_ptr;     /* TYPE_LANG_STRUCT via pointer */
    
    /* Undefined reference */
    struct undefined_type * GTY(()) undefined_ptr; /* TYPE_UNDEFINED */
};

#endif /* GTY_TEST_H */

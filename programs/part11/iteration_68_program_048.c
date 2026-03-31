/* gty-test.h - Test file for gengtype type categorization */

#ifndef GTY_TEST_H
#define GTY_TEST_H

/* Forward declaration for TYPE_UNDEFINED */
struct opaque;

/* TYPE_SCALAR */
typedef int GTY(()) scalar_type;

/* TYPE_STRING */
typedef const char * GTY(()) string_type;

/* TYPE_STRUCT */
struct GTY(()) test_struct {
    int scalar_field;           /* TYPE_SCALAR */
    const char *string_field;   /* TYPE_STRING */
    struct opaque *opaque_ptr;  /* TYPE_UNDEFINED via pointer */
};

/* TYPE_USER_STRUCT - user-defined struct with special handling */
typedef struct GTY(()) test_struct user_struct_type;

/* TYPE_UNION */
union GTY(()) test_union {
    int int_val;
    float float_val;
    void *ptr_val;
};

/* TYPE_POINTER */
typedef struct test_struct * GTY(()) struct_pointer;

/* TYPE_ARRAY */
typedef int GTY(()) int_array[10];
typedef struct test_struct GTY(()) struct_array[5];

/* TYPE_CALLBACK */
typedef void (* GTY(()) callback_func)(int, const char *);

/* TYPE_LANG_STRUCT - simulate language-specific struct */
#ifdef IN_LANG_SPECIFIC
struct GTY(()) lang_specific_struct {
    int lang_field;
};
#endif

/* Nested structures to ensure full processing */
struct GTY(()) container {
    /* TYPE_POINTER */
    struct test_struct *nested_ptr;
    
    /* TYPE_ARRAY */
    callback_func callbacks[3];
    
    /* TYPE_UNION */
    union test_union data;
    
    /* TYPE_CALLBACK as field */
    callback_func handler;
    
    /* TYPE_ARRAY of pointers */
    struct test_struct * GTY(()) ptr_array[4];
    
    /* Multi-dimensional array */
    int GTY(()) matrix[3][3];
};

/* Another structure with all types */
struct GTY(()) comprehensive {
    /* Basic types */
    scalar_type s;              /* TYPE_SCALAR */
    string_type str;            /* TYPE_STRING */
    
    /* Complex types */
    struct_pointer sp;          /* TYPE_POINTER */
    int_array arr;              /* TYPE_ARRAY */
    callback_func cb;           /* TYPE_CALLBACK */
    
    /* Embedded types */
    struct test_struct embedded; /* TYPE_STRUCT */
    union test_union choice;     /* TYPE_UNION */
    
    /* Pointer to undefined type */
    struct opaque *unknown;      /* TYPE_UNDEFINED */
    
    /* Array of callbacks */
    callback_func GTY(()) handlers[2];
    
    /* Pointer array */
    struct test_struct * GTY(()) pointers[3];
};

/* Template-like structure for edge cases */
struct GTY(()) node {
    struct node * GTY((skip)) next;  /* Pointer with skip option */
    struct node * GTY((chain_next ("%h.next"))) chain_next;
    const char * GTY((tag ("0"))) name;
    int GTY((desc ("%1"))) kind;
};

/* Union with pointers */
union GTY(()) ptr_union {
    void * GTY((tag ("0"))) generic;
    struct test_struct * GTY((tag ("1"))) specific;
    callback_func GTY((tag ("2"))) callback;
};

#endif /* GTY_TEST_H */

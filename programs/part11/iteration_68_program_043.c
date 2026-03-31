/* gty-test.h - Test file for gengtype type classification coverage */

#ifndef GTY_TEST_H
#define GTY_TEST_H

/* Forward declaration for TYPE_UNDEFINED test */
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
    int id;
    char *name;
} user_struct_type;

/* TYPE_UNION: union type */
union GTY(()) test_union {
    int as_int;
    char *as_string;
    void *as_ptr;
};

/* TYPE_POINTER: pointer type in struct */
struct GTY(()) pointer_container {
    struct basic_struct *GTY((skip)) struct_ptr;  /* TYPE_POINTER */
    user_struct_type *GTY((skip)) user_struct_ptr; /* TYPE_POINTER */
};

/* TYPE_ARRAY: array type in struct */
struct GTY(()) array_container {
    int GTY((length("array_len"))) *int_array;  /* TYPE_ARRAY */
    char *GTY((length("str_len"))) *str_array;  /* TYPE_ARRAY */
    int array_len;
    int str_len;
};

/* TYPE_CALLBACK: function pointer typedef */
typedef void (*GTY(()) callback_type)(int, char *);

/* Struct containing callback */
struct GTY(()) callback_container {
    callback_type GTY((skip)) callback;  /* TYPE_CALLBACK */
    void (*GTY((skip)) direct_callback)(void); /* Another callback */
};

/* TYPE_LANG_STRUCT: Simulating language-specific struct */
/* In GCC, these are marked with GTY((user)) or special tags */
struct GTY((user)) lang_specific_struct {
    void *data;
    int lang_tag;
};

/* Complex nested structure to test multiple classifications */
struct GTY(()) complex_type {
    /* Scalar fields */
    int count;
    unsigned long flags;
    
    /* String field */
    char *description;
    
    /* Pointer fields */
    struct basic_struct *nested_struct;
    union test_union *union_ptr;
    
    /* Array field */
    int GTY((length("item_count"))) *items;
    int item_count;
    
    /* Callback field */
    callback_type notify;
    
    /* Nested struct */
    struct GTY(()) inner_struct {
        int inner_id;
        char *inner_name;
    } inner;
    
    /* Union field */
    union GTY(()) data_union {
        int num;
        char *str;
        void *ptr;
    } data;
};

/* Another forward declaration to ensure TYPE_UNDEFINED is considered */
struct another_opaque;

/* Struct with undefined type pointer */
struct GTY(()) uses_undefined {
    struct opaque *opaque1;
    struct another_opaque *opaque2;
    void *generic_ptr;
};

#endif /* GTY_TEST_H */

#ifndef TEST_GENGYPE_H
#define TEST_GENGYPE_H

/* TYPE_UNDEFINED: Forward declaration that won't be defined */
struct undefined_struct GTY((tag("undefined")));

/* TYPE_SCALAR: Fundamental types with GTY markers */
typedef unsigned long my_scalar_t GTY((user));
typedef int another_scalar_t GTY((user));

/* TYPE_STRUCT: Standard C struct */
struct my_struct GTY((tag("my_struct"))) {
    my_scalar_t field1;
    another_scalar_t field2;
    struct my_struct *next GTY((skip));  /* TYPE_POINTER */
};

/* TYPE_USER_STRUCT: Marked for special user handling */
struct user_handled_struct GTY((user)) {
    int user_field1;
    char user_field2;
};

/* TYPE_UNION: Union with GTY-tagged members */
union my_union GTY((tag("my_union"))) {
    my_scalar_t as_scalar;
    struct my_struct *as_struct_ptr GTY((skip));
    char *as_string GTY((length("strlen($) + 1")));
};

/* TYPE_ARRAY: Various array types */
struct array_container GTY((tag("array_container"))) {
    /* Fixed-size array */
    int fixed_array[10] GTY((skip));
    
    /* Zero-length array */
    char zero_length_array[0] GTY((skip));
    
    /* Array with length attribute */
    struct my_struct *variable_array GTY((length("$->array_len")));
    int array_len;
};

/* TYPE_STRING: String type with length attribute */
struct string_container GTY((tag("string_container"))) {
    char *string_field GTY((length("strlen($) + 1")));
    const char *const_string GTY((length("strlen($) + 1")));
};

/* TYPE_CALLBACK: Function pointer with callback attribute */
typedef void (*callback_func_t)(int, char *) GTY((callback));

struct callback_container GTY((tag("callback_container"))) {
    callback_func_t handler;
    void (*direct_callback)(struct my_struct *) GTY((callback));
};

/* TYPE_LANG_STRUCT: Language-specific struct */
struct lang_specific_struct GTY((tag("lang_struct"), lang_struct (1))) {
    int lang_field1;
    void *lang_field2 GTY((skip));
};

/* Nested structures for complex type graph */
struct outer_struct GTY((tag("outer_struct"))) {
    /* Nested struct */
    struct inner_nested GTY((tag("inner_nested"))) {
        int nested_field;
        struct outer_struct *parent GTY((skip));  /* Circular reference */
    } nested;
    
    /* Nested union */
    union {
        int union_option1;
        struct my_struct *union_option2 GTY((skip));
    } choice GTY((tag("choice_union")));
    
    /* Pointer to forward-declared type (TYPE_UNDEFINED until defined) */
    struct undefined_struct *undefined_ptr GTY((skip));
    
    /* Array of pointers */
    struct my_struct *ptr_array[5] GTY((skip));
    
    /* Multi-dimensional array */
    int matrix[3][4] GTY((skip));
};

/* TYPE_POINTER: Various pointer types in different contexts */
typedef struct my_struct *struct_ptr_t GTY((skip));

/* Now define the previously forward-declared struct to complete TYPE_UNDEFINED */
struct undefined_struct GTY((tag("undefined_struct"))) {
    int now_defined;
    struct outer_struct *link GTY((skip));
};

/* Function pointer typedefs */
typedef int (*comparator_t)(const void *, const void *) GTY((callback));

#endif /* TEST_GENGYPE_H */

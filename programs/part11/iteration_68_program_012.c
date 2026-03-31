/* gty-test.h - Test file for gengtype type classification */
#ifndef GTY_TEST_H
#define GTY_TEST_H

/* Forward declaration for undefined type */
struct opaque;

/* TYPE_UNDEFINED: Forward-declared opaque struct */
extern struct opaque *global_opaque_ptr;

/* TYPE_CALLBACK: Function pointer typedef */
typedef void (*callback_func)(int, char *) GTY((callback));

/* TYPE_STRUCT: Basic struct with GTY annotation */
struct GTY(()) test_struct {
    /* TYPE_SCALAR: Plain scalar member */
    int scalar_member;
    
    /* TYPE_STRING: String member */
    const char * GTY((length("strlen(%h.string_member) + 1"))) string_member;
    
    /* TYPE_POINTER: Pointer to another struct */
    struct test_struct * GTY((skip)) pointer_member;
    
    /* TYPE_ARRAY: Array member */
    int GTY((length("array_length"))) array_member[10];
    
    /* TYPE_UNDEFINED: Pointer to undefined type */
    struct opaque *opaque_ptr;
    
    /* TYPE_CALLBACK: Callback function pointer */
    callback_func callback;
};

/* TYPE_UNION: Union with GTY annotation */
union GTY((desc("%0.type"))) test_union {
    int type;
    struct test_struct * GTY((tag("1"))) struct_ptr;
    int GTY((tag("2"))) int_value;
    char * GTY((tag("3"))) string_value;
};

/* TYPE_USER_STRUCT: Using typedef with struct */
typedef struct GTY(()) {
    int id;
    char *name;
} user_struct_t;

/* TYPE_LANG_STRUCT: Language-specific structure */
#ifdef LANGUAGE_HOOKS
struct GTY(()) lang_struct {
    int lang_specific;
    void *lang_data;
};
#endif

/* TYPE_POINTER: Global pointer */
extern struct test_struct * GTY((skip)) global_struct_ptr;

/* TYPE_ARRAY: Global array */
extern int GTY((length("global_array_len"))) global_array[100];

/* Nested structures for complex testing */
struct GTY(()) container {
    /* TYPE_UNION: Union member */
    union test_union data;
    
    /* TYPE_STRUCT: Struct member */
    struct test_struct nested;
    
    /* TYPE_ARRAY of pointers */
    struct test_struct * GTY((length("ptr_count"))) ptr_array[5];
    
    /* TYPE_CALLBACK in struct */
    callback_func handlers[3];
};

/* Variadic callback type */
typedef void (*variadic_callback)(int, ...) GTY((callback));

#endif /* GTY_TEST_H */

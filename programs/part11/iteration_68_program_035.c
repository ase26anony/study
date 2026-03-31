/* gty-test.h - Test file for gengtype type classification coverage */

#ifndef GTY_TEST_H
#define GTY_TEST_H

/* Forward declaration for TYPE_UNDEFINED test */
struct opaque;

/* TYPE_CALLBACK: Function pointer typedef with GTY annotation */
typedef void (*callback_func) (int, const char *) GTY((callback));

/* TYPE_STRUCT: Basic struct with GTY annotation */
struct GTY(()) base_struct {
    /* TYPE_SCALAR: Plain scalar member */
    int scalar_member;
    
    /* TYPE_STRING: String member */
    const char * GTY((length("strlen(%h.string_member) + 1"))) string_member;
    
    /* TYPE_POINTER: Pointer to another struct */
    struct base_struct *next;
    
    /* TYPE_ARRAY: Array member */
    int GTY((length("10"))) array_member[10];
    
    /* TYPE_UNDEFINED: Opaque pointer */
    struct opaque *opaque_ptr;
    
    /* TYPE_CALLBACK: Function pointer field */
    callback_func callback;
};

/* TYPE_UNION: Union with GTY annotation */
union GTY(()) test_union {
    int as_int;
    float as_float;
    struct base_struct * GTY((tag("0"))) as_struct;
    void *as_pointer;
};

/* TYPE_USER_STRUCT: Struct with user-defined GC markers */
struct GTY((user)) user_struct {
    int id;
    void *data;
    void (*marker) (void *);
};

/* TYPE_LANG_STRUCT: Language-specific struct */
struct GTY((desc("%0.lang_tag"))) lang_struct {
    int lang_tag;
    union test_union value;
};

/* Nested structures for complex testing */
struct GTY(()) container {
    /* Multiple TYPE_POINTER cases */
    struct base_struct *first;
    struct base_struct **second;  /* Pointer to pointer */
    
    /* TYPE_ARRAY of pointers */
    struct base_struct * GTY((length("5"))) ptr_array[5];
    
    /* TYPE_ARRAY of structs */
    struct base_struct GTY((length("3"))) struct_array[3];
    
    /* TYPE_UNION member */
    union test_union data;
    
    /* TYPE_CALLBACK array */
    callback_func GTY((length("2"))) callbacks[2];
};

/* Additional test for TYPE_STRING variations */
struct GTY(()) string_container {
    /* Various string types */
    char * GTY((length("strlen(%h.dynamic_str) + 1"))) dynamic_str;
    const char *constant_str;
    char fixed_str[50];
};

#endif /* GTY_TEST_H */

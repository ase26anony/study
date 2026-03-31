/* test-gty.h - Header file with GTY annotations for gengtype testing */

#ifndef TEST_GTY_H
#define TEST_GTY_H

/* Forward declarations */
struct my_struct;
union my_union;

/* TYPE_STRUCT: Basic struct with GTY annotation */
struct my_struct GTY(()) {
    int id;
    char *name;
};

/* TYPE_UNION: Basic union with GTY annotation */
union my_union GTY(()) {
    int int_val;
    float float_val;
    void *ptr_val;
};

/* TYPE_POINTER: Struct containing pointer members */
struct pointer_container GTY(()) {
    /* Regular pointer */
    struct my_struct * GTY((skip)) struct_ptr;
    
    /* Pointer to union */
    union my_union * GTY((skip)) union_ptr;
    
    /* Void pointer */
    void * GTY((skip)) void_ptr;
};

/* TYPE_ARRAY: Struct with array members */
struct array_container GTY(()) {
    /* Fixed-size array */
    int GTY((length("10"))) fixed_array[10];
    
    /* Array of pointers */
    struct my_struct * GTY((skip)) ptr_array[5];
    
    /* Multi-dimensional array */
    float GTY((length("3*4"))) matrix[3][4];
};

/* TYPE_SCALAR: Struct with scalar types */
struct scalar_container GTY(()) {
    /* Various scalar types with GTY */
    int GTY((skip)) int_field;
    long GTY((skip)) long_field;
    unsigned GTY((skip)) unsigned_field;
    double GTY((skip)) double_field;
    enum { RED, GREEN, BLUE } GTY((skip)) color_field;
};

/* TYPE_STRING: Struct with string members */
struct string_container GTY(()) {
    /* Regular string pointer */
    const char * GTY((skip)) name;
    
    /* String with length attribute */
    char * GTY((length("strlen($)"))) dynamic_string;
    
    /* Array of strings */
    const char * GTY((skip)) string_array[3];
};

/* TYPE_CALLBACK: Callback function type */
typedef void (*callback_func)(int, void*) GTY((callback));

/* Struct using callback type */
struct callback_container GTY(()) {
    callback_func GTY((skip)) handler;
    void * GTY((skip)) user_data;
};

/* Complex nested structure for type graph testing */
struct complex_node GTY(()) {
    int value;
    struct complex_node * GTY((skip)) next;
    struct complex_node * GTY((skip)) prev;
    struct complex_node * GTY((skip)) children[5];
    union my_union GTY((tag("0"))) data;
};

/* Template-like macro for generating multiple struct types */
#define DEF_PAIR(T, NAME) \
    struct NAME GTY(()) { \
        T first; \
        T second; \
    }

/* Instantiate template-like structs */
DEF_PAIR(int, int_pair);
DEF_PAIR(float, float_pair);
DEF_PAIR(struct my_struct*, struct_ptr_pair);

/* Union containing struct and pointer */
union mixed_union GTY(()) {
    struct my_struct GTY((tag("1"))) as_struct;
    struct my_struct * GTY((skip)) as_pointer;
    int as_int;
};

/* Language-specific structure hook (simulating Tree nodes) */
struct lang_struct GTY((tag("TS_VAR_DECL"))) {
    int decl_uid;
    const char * GTY((skip)) decl_name;
    struct lang_struct * GTY((skip)) chain;
};

/* User-defined struct type forward declaration */
struct user_defined_type;

/* Container referencing user-defined type */
struct user_container GTY(()) {
    struct user_defined_type * GTY((skip)) user_data;
    int metadata;
};

#endif /* TEST_GTY_H */

#ifndef TEST_GTY_H
#define TEST_GTY_H

/* TYPE_STRUCT: Basic struct with GTY annotation */
struct GTY(()) base_struct {
    int id;
    char data;
};

/* TYPE_UNION: Basic union with GTY annotation */
union GTY(()) data_union {
    int int_val;
    float float_val;
    void* ptr_val;
};

/* TYPE_POINTER: Struct containing pointers */
struct GTY(()) pointer_container {
    struct base_struct* GTY((skip)) struct_ptr;  /* TYPE_POINTER */
    void* GTY((skip)) generic_ptr;               /* TYPE_POINTER */
    int* GTY((skip)) int_ptr;                    /* TYPE_POINTER */
};

/* TYPE_ARRAY: Struct with fixed-size array */
struct GTY(()) array_container {
    int GTY((length("10"))) fixed_array[10];     /* TYPE_ARRAY */
    char GTY((length("256"))) buffer[256];       /* TYPE_ARRAY */
};

/* TYPE_SCALAR: Various scalar types */
struct GTY(()) scalar_container {
    long GTY((skip)) counter;                    /* TYPE_SCALAR */
    unsigned GTY((skip)) flags;                  /* TYPE_SCALAR */
    double GTY((skip)) value;                    /* TYPE_SCALAR */
};

/* TYPE_STRING: String types */
struct GTY(()) string_container {
    const char* GTY((skip)) name;                /* TYPE_STRING */
    char* GTY((skip)) dynamic_str;               /* TYPE_STRING */
};

/* TYPE_CALLBACK: Callback function type */
typedef void (*callback_fn)(int, void*) GTY((callback));  /* TYPE_CALLBACK */

struct GTY(()) callback_container {
    callback_fn GTY((skip)) handler;             /* TYPE_CALLBACK */
};

/* Complex nested type graph */
struct GTY(()) node {
    int value;
    struct node* GTY((skip)) next;               /* Recursive pointer */
    struct node* GTY((skip)) prev;               /* Recursive pointer */
};

/* Union containing struct and pointer */
union GTY(()) complex_union {
    struct base_struct nested_struct;
    struct pointer_container* GTY((skip)) container_ptr;
    callback_fn action;
};

/* Template-like macro for generating multiple types */
#define DEF_PAIR(T) struct pair_##T { T first; T second; } GTY(())

DEF_PAIR(int);
DEF_PAIR(double);
DEF_PAIR(struct base_struct*);

/* Forward declaration for mutual recursion */
struct GTY(()) forward_decl;
struct GTY(()) recursive_container {
    struct forward_decl* GTY((skip)) fwd_ptr;
};

struct GTY(()) forward_decl {
    int data;
    struct recursive_container* GTY((skip)) back_ptr;
};

/* Language-specific structure simulation */
#ifdef __cplusplus
/* Simulating Tree node for TYPE_LANG_STRUCT */
struct GTY((tag("TS_VAR_DECL"))) lang_struct {
    int decl_uid;
    void* GTY((skip)) lang_specific;
};
#endif

/* User-defined type marker (requires gtype-desc.cc) */
struct user_defined_type;

#endif /* TEST_GTY_H */

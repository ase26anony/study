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
    struct base_struct* GTY((skip)) next;  /* TYPE_POINTER */
    void* GTY((skip)) raw_ptr;             /* TYPE_POINTER */
    const char* GTY((skip)) description;   /* TYPE_STRING via pointer */
};

/* TYPE_ARRAY: Struct with fixed-size array */
struct GTY(()) array_container {
    int GTY((length("10"))) fixed_array[10];  /* TYPE_ARRAY */
    struct base_struct* GTY((length("5"))) ptr_array[5]; /* TYPE_ARRAY of pointers */
};

/* TYPE_SCALAR: Direct scalar types with GTY */
typedef long GTY((skip)) counter_t;
typedef unsigned int GTY((skip)) flags_t;

/* TYPE_STRING: String types */
typedef const char* GTY((skip)) string_t;

/* TYPE_CALLBACK: Callback function type */
typedef void (*callback_func)(int, void*) GTY((callback));

/* Complex nested structure for type graph */
struct GTY(()) complex_node {
    int value;
    struct complex_node* GTY((skip)) left;   /* Recursive pointer */
    struct complex_node* GTY((skip)) right;  /* Recursive pointer */
    union data_union GTY((tag("union_type"))) data; /* Embedded union */
};

/* Template-like macro for multiple type instances */
#define DEF_PAIR(T) struct pair_##T { \
    T first; \
    T second; \
} GTY(())

/* Instantiate template-like types */
DEF_PAIR(int);
DEF_PAIR(struct base_struct*);

/* Forward declaration for user-defined type */
struct user_defined_type;

/* Struct referencing user-defined type */
struct GTY(()) user_container {
    struct user_defined_type* GTY((skip)) user_data;
};

/* Language-specific structure simulation */
struct GTY((tag("TS_VAR_DECL"))) lang_specific {
    int decl_uid;
    const char* GTY((skip)) name;
    struct lang_specific* GTY((skip)) chain;
};

#endif /* TEST_GTY_H */

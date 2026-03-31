/* test-gty.h - Header file with GTY annotations for gengtype testing */

#ifndef TEST_GTY_H
#define TEST_GTY_H

/* TYPE_STRUCT: Basic struct with GTY annotation */
struct my_struct GTY(()) {
    int x;
    double y;
};

/* TYPE_UNION: Basic union with GTY annotation */
union my_union GTY(()) {
    int int_val;
    double double_val;
    void* ptr_val;
};

/* TYPE_POINTER: Will be used in nested struct */
struct pointer_container GTY(()) {
    struct my_struct* GTY((skip)) struct_ptr;  /* TYPE_POINTER */
    union my_union* GTY((skip)) union_ptr;     /* TYPE_POINTER */
};

/* TYPE_ARRAY: Fixed-size array */
struct array_container GTY(()) {
    int GTY((length("10"))) fixed_array[10];  /* TYPE_ARRAY */
    struct my_struct GTY((length("5"))) struct_array[5];
};

/* TYPE_SCALAR: Scalar types with GTY */
struct scalar_container GTY(()) {
    long GTY((skip)) counter;      /* TYPE_SCALAR */
    unsigned GTY((skip)) flags;    /* TYPE_SCALAR */
    short GTY((skip)) small;       /* TYPE_SCALAR */
};

/* TYPE_STRING: String types */
struct string_container GTY(()) {
    const char* GTY((skip)) name;          /* TYPE_STRING */
    char* GTY((skip)) mutable_string;      /* TYPE_STRING */
};

/* TYPE_CALLBACK: Callback function type */
typedef void (*callback_fn)(int) GTY((callback));  /* TYPE_CALLBACK */

struct callback_container GTY(()) {
    callback_fn GTY((skip)) handler;
};

/* Complex nested type for recursive processing */
struct nested_container GTY(()) {
    struct pointer_container* GTY((skip)) ptr_container;
    struct array_container array_container;
    struct nested_container* GTY((skip)) next;  /* Self-referential pointer */
};

/* Template-like macro for generating multiple types */
#define DEF_PAIR(T) struct pair_##T { T first; T second; } GTY(())

DEF_PAIR(int);
DEF_PAIR(double);
DEF_PAIR(struct my_struct*);

/* Forward declaration for user-defined type */
struct user_defined_type;

/* Container referencing user-defined type */
struct user_container GTY(()) {
    struct user_defined_type* GTY((skip)) user_data;
};

/* Language-specific structure simulation */
struct lang_specific GTY((tag("TS_VAR_DECL"))) {  /* TYPE_LANG_STRUCT hint */
    int decl_code;
    void* GTY((skip)) lang_specific_data;
};

#endif /* TEST_GTY_H */

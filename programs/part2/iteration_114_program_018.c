#ifndef TEST_GENGYPE_H
#define TEST_GENGYPE_H

/* TYPE_UNDEFINED: Forward declaration without definition */
struct undefined_struct GTY((tag("UNDEFINED")));

/* TYPE_SCALAR: Fundamental types with GTY markers */
typedef unsigned long my_scalar_t GTY((user));
typedef int another_scalar_t GTY(());

/* TYPE_STRUCT: Standard C structs */
struct base_struct GTY((tag("BASE"))) {
    my_scalar_t value;
    struct undefined_struct* forward_ptr;  /* Pointer to undefined type */
};

/* TYPE_USER_STRUCT: Struct with user attribute */
struct user_handled_struct GTY((user)) {
    int user_data;
    char* user_name;
};

/* TYPE_UNION: Union with GTY members */
union data_union GTY((tag("DATA_UNION"))) {
    int int_val;
    char* str_val;
    struct base_struct* struct_ptr;
};

/* TYPE_POINTER: Struct containing various pointers */
struct pointer_container GTY((tag("PTR_CONT"))) {
    struct base_struct* next GTY((skip));          /* Pointer field */
    union data_union* union_ptr;                   /* Another pointer */
    struct pointer_container* self_ptr;            /* Recursive pointer */
};

/* TYPE_ARRAY: Struct containing arrays */
struct array_container GTY((tag("ARRAY_CONT"))) {
    int fixed_array[10];                           /* Fixed-size array */
    struct base_struct* var_array GTY((length("var_len"))); /* Variable array */
    int var_len;
    char zero_array[0];                            /* Zero-length array */
};

/* TYPE_STRING: String type with length attribute */
struct string_container GTY((tag("STR_CONT"))) {
    char* dynamic_string GTY((length("str_len"))); /* String type */
    int str_len;
    const char* static_string;                     /* Another string */
};

/* TYPE_CALLBACK: Function pointer with callback attribute */
typedef void (*callback_func_t)(int, char*) GTY((callback));

struct callback_container GTY((tag("CALLBACK_CONT"))) {
    callback_func_t handler GTY((skip));           /* Callback field */
    void (*plain_func_ptr)(void);                  /* Regular function pointer */
};

/* TYPE_LANG_STRUCT: Language-specific struct */
struct lang_specific_struct GTY((tag("LANG_STRUCT"), lang_struct(1))) {
    int lang_data;
    void* lang_handle;
};

/* Nested struct definition for complexity */
struct outer_container GTY((tag("OUTER"))) {
    struct {
        int nested_data;
        union {
            int nested_int;
            char nested_char;
        } nested_union;
    } nested_struct;
    
    struct array_container arrays;
    struct string_container strings;
};

/* Now define the previously undefined struct to create TYPE_UNDEFINED case */
/* (But leave it commented to keep it undefined for coverage) */
/*
struct undefined_struct {
    int finally_defined;
};
*/

#endif /* TEST_GENGYPE_H */

#ifndef TEST_GENGTYPE_H
#define TEST_GENGTYPE_H

/* TYPE_UNDEFINED: Forward declaration that will remain undefined */
struct undefined_struct;

/* TYPE_SCALAR: Fundamental types with GTY markers */
typedef unsigned long my_scalar_t GTY(());
typedef int another_scalar_t GTY(());
typedef double float_scalar_t GTY(());

/* TYPE_STRUCT: Standard C structs */
struct GTY(()) base_struct {
    my_scalar_t field1;
    another_scalar_t field2;
};

/* TYPE_USER_STRUCT: Struct with user attribute */
struct GTY((user)) user_defined_struct {
    int user_data;
    void* user_pointer;
};

/* TYPE_UNION: Union with GTY members */
union GTY(()) data_union {
    int int_val;
    float float_val;
    char* string_val;
    struct base_struct* struct_ptr;
};

/* TYPE_POINTER: Pointer types and recursive references */
struct GTY(()) node {
    int value;
    struct node* GTY((skip)) next;  /* Skip prevents infinite recursion */
    struct undefined_struct* GTY((skip)) undefined_ptr;  /* Pointer to undefined type */
};

/* TYPE_ARRAY: Array types */
struct GTY(()) array_container {
    int fixed_array[10];
    struct base_struct* GTY((length("variable_len"))) variable_array[];
    int variable_len;
    char zero_length_array[0];
};

/* TYPE_STRING: String type with length attribute */
struct GTY(()) string_container {
    char* GTY((length("str_len"))) dynamic_string;
    int str_len;
    const char* GTY((length("const_len"))) const_string;
    int const_len;
};

/* TYPE_CALLBACK: Function pointer with callback attribute */
typedef void (*callback_func_t)(int, void*) GTY((callback));

struct GTY(()) callback_container {
    callback_func_t handler;
    void* GTY((skip)) user_data;
};

/* TYPE_LANG_STRUCT: Language-specific struct */
struct GTY((lang_struct (1))) lang_specific {
    int lang_data;
    void* lang_pointer;
};

/* Complex nested types for interdependencies */
struct GTY(()) outer_struct {
    /* Nested struct */
    struct GTY(()) inner_struct {
        int inner_data;
        struct outer_struct* GTY((skip)) parent;
    } nested;
    
    /* Nested union */
    union GTY(()) inner_union {
        int option_a;
        float option_b;
        struct inner_struct* GTY((skip)) option_c;
    } choice;
    
    /* Array of pointers */
    struct inner_struct* GTY((length("child_count"))) children[];
    int child_count;
    
    /* String field */
    char* GTY((length("name_len"))) name;
    int name_len;
    
    /* Callback field */
    callback_func_t notify;
    
    /* Pointer to language struct */
    struct lang_specific* GTY((skip)) lang_info;
};

/* Another forward declaration for circular reference */
struct forward_declared;

struct GTY(()) uses_forward_decl {
    int id;
    struct forward_declared* GTY((skip)) forward_ptr;
};

struct GTY(()) forward_declared {
    int data;
    struct uses_forward_decl* GTY((skip)) back_ptr;
};

/* Mixed type container */
struct GTY(()) type_mixer {
    /* Scalar */
    my_scalar_t scalar_field;
    
    /* Struct */
    struct base_struct struct_field;
    
    /* Union */
    union data_union union_field;
    
    /* Pointer */
    struct node* pointer_field;
    
    /* Array */
    int array_field[5];
    
    /* String */
    char* GTY((length("mixer_str_len"))) string_field;
    int mixer_str_len;
    
    /* Callback */
    callback_func_t callback_field;
    
    /* Nested struct */
    struct GTY(()) {
        int anonymous_data;
        char* GTY((length("anon_len"))) anonymous_string;
        int anon_len;
    } anonymous_field;
    
    /* Pointer to user struct */
    struct user_defined_struct* GTY((skip)) user_struct_ptr;
    
    /* Pointer to lang struct */
    struct lang_specific* GTY((skip)) lang_struct_ptr;
};

#endif /* TEST_GENGTYPE_H */

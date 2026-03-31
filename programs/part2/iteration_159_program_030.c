#ifndef TEST_GTY_H
#define TEST_GTY_H

/* TYPE_STRUCT - Basic struct with GTY annotation */
struct GTY(()) base_struct {
    int id;
    char *GTY((skip)) name;
};

/* TYPE_UNION - Basic union with GTY annotation */
union GTY(()) data_union {
    int int_val;
    float float_val;
    char *GTY((skip)) string_val;
};

/* TYPE_POINTER - Struct containing pointers */
struct GTY(()) pointer_container {
    struct base_struct *GTY((tag("0"))) ptr_to_struct;
    void *GTY((skip)) opaque_ptr;
    union data_union *GTY((skip)) union_ptr;
};

/* TYPE_ARRAY - Struct with arrays */
struct GTY(()) array_container {
    int GTY((length("fixed_size"))) fixed_arr[10];
    struct base_struct *GTY((length("var_size"))) var_arr[1];
    char GTY((length("str_len"))) string_arr[256];
    int fixed_size;
    int var_size;
    int str_len;
};

/* TYPE_SCALAR - Direct scalar type annotation */
typedef long GTY((skip)) counter_type;

/* TYPE_STRING - String type */
typedef const char *GTY((skip)) string_type;

/* TYPE_CALLBACK - Callback function type */
typedef void (*GTY((callback)) callback_func)(int, void*);

/* Complex nested structure for type graph */
struct GTY(()) complex_node {
    int value;
    struct complex_node *GTY((skip)) next;
    struct complex_node *GTY((skip)) prev;
    union data_union data;
};

/* Template-like macro for multiple type instances */
#define DEF_PAIR(T) struct pair_##T { \
    T first; \
    T second; \
} GTY(())

/* Instantiate template-like types */
DEF_PAIR(int);
DEF_PAIR(struct base_struct*);

/* Forward declaration for mutual recursion */
struct GTY(()) forward_decl;
struct GTY(()) recursive_struct {
    int id;
    struct forward_decl *GTY((skip)) fwd_ptr;
};

struct GTY(()) forward_decl {
    struct recursive_struct *GTY((skip)) back_ptr;
};

/* Language-specific structure simulation */
struct GTY((tag("TS_VAR_DECL"))) lang_specific {
    int decl_uid;
    void *GTY((skip)) lang_specific_data;
};

#endif /* TEST_GTY_H */

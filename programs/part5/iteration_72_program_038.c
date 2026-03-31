#ifndef GTY_TEST_TYPES_H
#define GTY_TEST_TYPES_H

/* Basic structure type - TYPE_STRUCT */
struct GTY(()) base_struct {
    int id;
    char *GTY((skip)) name;  /* Pointer with skip option */
};

/* Union type - TYPE_UNION */
union GTY(()) data_union {
    int int_val;
    float float_val;
    void *GTY((skip)) ptr_val;
    struct base_struct *GTY((tag("0"))) struct_ptr;
};

/* Recursive structure for TYPE_POINTER traversal */
struct GTY(()) recursive_node {
    int value;
    struct recursive_node *GTY((skip)) next;  /* Self-referential pointer */
    struct recursive_node *GTY((chain_next("next"))) prev;
};

/* Array type definition */
typedef int GTY(()) int_array[10];
typedef struct base_struct *GTY(()) struct_ptr_array[5];

/* Callback type - TYPE_CALLBACK */
typedef void (*GTY(()) callback_func)(int, const char*);

/* User struct type via typedef - TYPE_USER_STRUCT */
typedef struct base_struct GTY(()) base_struct_t;

/* Scalar type in container */
struct GTY(()) scalar_container {
    long GTY((skip)) scalar_value;
    unsigned GTY((length("len"))) count;
    int len;
};

/* String type handling - TYPE_STRING */
struct GTY(()) string_container {
    const char *GTY((length("strlen"))) text;
    int strlen;
};

/* Complex nested structure */
struct GTY(()) complex_nested {
    union data_union data;
    struct recursive_node *node_chain;
    int_array numbers;
    callback_func GTY((skip)) handler;
};

/* Forward declaration for mutual recursion */
struct GTY(()) forward_decl_struct;
struct GTY(()) another_struct;

/* Mutual recursion */
struct GTY(()) forward_decl_struct {
    int id;
    struct another_struct *GTY((skip)) partner;
};

struct GTY(()) another_struct {
    int id;
    struct forward_decl_struct *GTY((skip)) partner;
};

/* Conditional compilation with GTY */
#ifdef USE_GTY_OPTIONS
struct GTY((for_user)) conditional_struct {
    int x;
    char *GTY((skip, tag("1"))) y;  /* Multiple options */
};
#else
struct conditional_struct {
    int x;
    char *y;
};
#endif

/* Array with length specifier */
struct GTY(()) array_with_length {
    int count;
    struct base_struct *GTY((length("count"))) items;
};

/* Language-like structure (simulating TYPE_LANG_STRUCT) */
struct GTY(()) lang_struct_sim {
    int lang_specific;
    void *GTY((skip)) lang_data;
};

#endif /* GTY_TEST_TYPES_H */

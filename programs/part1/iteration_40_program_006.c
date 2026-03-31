/* test_gty.h - Test header with GTY annotations for all type categories */

#ifndef TEST_GTY_H
#define TEST_GTY_H

/* Define GTY macro if not already defined */
#ifndef GTY
#define GTY(x) __attribute__((gty(x)))
#endif

/* Dummy definitions for GCC internal types */
typedef int tree;
typedef void* rtx;
typedef void* gimple;

/* 1. TYPE_UNDEFINED: Forward declared struct without definition */
struct GTY(()) undefined_struct;

/* 2. TYPE_SCALAR: Basic scalar type with user attribute */
typedef int GTY((user)) my_scalar_t;

/* 3. TYPE_STRING: String type with length attribute */
const char * GTY((length("strlen(%h.my_string)"))) my_string;

/* 4. TYPE_STRUCT: Regular struct with tag attribute */
struct GTY((tag("my_struct_tag"))) my_struct {
    int field1;
    char * GTY((skip)) field2;
    tree field3;  /* Use dummy GCC type */
};

/* 5. TYPE_USER_STRUCT: Typedef of struct with user attribute */
typedef struct my_struct GTY((user)) my_user_struct_t;

/* 6. TYPE_UNION: Union with desc attribute for discrimination */
union GTY((desc("%0.a"))) my_union {
    int a;
    char * GTY((skip)) b;
    struct my_struct * GTY((skip)) c;
};

/* 7. TYPE_POINTER: Pointer to struct with skip attribute */
struct my_struct * GTY((skip)) my_pointer;

/* 8. TYPE_ARRAY: Array with length attribute */
int GTY((length("10"))) my_array[10];

/* 9. TYPE_CALLBACK: Function pointer type with user attribute */
typedef void (*GTY((user)) my_callback_fn)(int, tree);

/* 10. TYPE_LANG_STRUCT: Language-specific structure with special attribute */
struct GTY((special("lang_struct"), desc("%1.kind"))) lang_specific_struct {
    int kind;
    union GTY((desc("%0.kind"))) {
        int ival;
        char *sval;
        struct my_struct *pval;
    } GTY((tag("0"))) u;
};

/* Additional complex types to ensure thorough parsing */
struct GTY(()) container {
    /* Mix of different types */
    my_scalar_t scalar_field;
    struct my_struct * GTY((skip)) ptr_field;
    int GTY((length("5"))) array_field[5];
    union my_union union_field;
    my_callback_fn callback_field;
};

/* Pointer chain to exercise pointer type counting */
struct GTY(()) pointer_chain {
    struct pointer_chain * GTY((skip)) next;
    struct container * GTY((skip)) data;
};

/* Array of pointers */
struct my_struct * GTY((length("8"))) ptr_array[8];

/* Nested struct with union */
struct GTY((tag("nested"))) nested_struct {
    int id;
    union GTY((desc("%0.id"))) {
        int x;
        double y;
        char *z;
    } value;
};

#endif /* TEST_GTY_H */

/* test_gty.h - Comprehensive GTY annotation test for gengtype coverage */

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

/* ===== TYPE_UNDEFINED ===== */
/* Forward declaration without definition */
struct GTY(()) undefined_struct;

/* ===== TYPE_SCALAR ===== */
typedef int GTY((user)) my_scalar_t;

/* ===== TYPE_STRING ===== */
const char * GTY((length)) my_string;

/* ===== TYPE_STRUCT ===== */
struct GTY((tag("my_struct"))) my_struct {
    int field;
    struct my_struct * GTY((skip)) next;
};

/* ===== TYPE_USER_STRUCT ===== */
typedef struct my_struct GTY((user)) my_user_struct_t;

/* ===== TYPE_UNION ===== */
union GTY((desc("0"))) my_union {
    int a;
    char * GTY((skip)) b;
    double c;
};

/* ===== TYPE_POINTER ===== */
struct my_struct * GTY((skip)) my_pointer;
tree GTY((user)) my_tree_pointer;

/* ===== TYPE_ARRAY ===== */
int GTY((length)) my_array[10];
struct my_struct GTY((tag("array_struct"))) struct_array[5];

/* ===== TYPE_CALLBACK ===== */
typedef void (*GTY((user)) my_callback_fn)(int);
my_callback_fn GTY((skip)) callback_ptr;

/* ===== TYPE_LANG_STRUCT ===== */
/* Language-specific structure with special marker */
struct GTY((special("lang_struct"))) lang_specific_struct {
    int lang_code;
    union {
        tree GTY((tag("0"))) t;
        rtx GTY((tag("1"))) r;
    } GTY((desc("%1.lang_code"))) u;
};

/* Additional complex types to ensure thorough parsing */

/* Nested struct with pointer chain */
struct GTY((tag("nested"))) nested_struct {
    struct my_struct * GTY((skip)) first;
    struct nested_struct * GTY((skip)) next;
    int GTY((length)) counts[5];
};

/* Union with callback */
union GTY((desc("type"))) complex_union {
    int type;
    my_callback_fn GTY((skip)) handler;
    const char * GTY((length)) message;
};

/* Array of pointers */
tree * GTY((user)) tree_ptr_array[8];

/* Struct containing array of structs */
struct GTY((tag("container"))) container_struct {
    struct my_struct GTY((tag("items"))) items[3];
    int count;
};

/* Forward declared struct that gets defined later */
struct GTY(()) late_defined_struct;

/* Definition of the forward declared struct */
struct GTY((tag("late"))) late_defined_struct {
    int value;
    struct late_defined_struct * GTY((skip)) chain;
};

/* Multiple levels of indirection */
struct GTY((tag("indirect"))) indirect_struct {
    struct my_struct ** GTY((skip)) double_ptr;
    struct indirect_struct * GTY((skip)) self_ref;
};

#endif /* TEST_GTY_H */

/* test_gty.h - Comprehensive GTY annotation test file */
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
const char * GTY((length(strlen("%s") + 1))) my_string;

/* ===== TYPE_STRUCT ===== */
struct GTY((tag("my_struct"))) my_struct {
    int field;
    struct my_struct *GTY((skip)) next;
};

/* ===== TYPE_USER_STRUCT ===== */
typedef struct my_struct GTY((user)) my_user_struct_t;

/* ===== TYPE_UNION ===== */
union GTY((desc("%d"))) my_union {
    int a;
    char * GTY((skip)) b;
    double c;
};

/* ===== TYPE_POINTER ===== */
struct my_struct * GTY((skip)) my_pointer;
tree GTY((user)) my_tree_pointer;

/* ===== TYPE_ARRAY ===== */
int GTY((length("10"))) my_array[10];
struct my_struct * GTY((length("5"))) struct_array[5];

/* ===== TYPE_CALLBACK ===== */
typedef void (*GTY((user)) my_callback_fn)(int);
my_callback_fn GTY((user)) callback_ptr;

/* ===== TYPE_LANG_STRUCT ===== */
/* Language-specific structure with special marker */
struct GTY((special("lang_struct"))) lang_specific_struct {
    int lang_code;
    union {
        tree t;
        rtx r;
    } GTY((desc("lang_code"))) u;
};

/* Additional complex types to ensure thorough parsing */

/* Nested struct with array */
struct GTY(()) container {
    struct my_struct GTY((tag("nested"))) nested;
    int GTY((length("container_len"))) dynamic_array[1];
};

/* Pointer chain */
typedef struct GTY(()) pointer_chain {
    struct pointer_chain *GTY((skip)) next;
    void *GTY((user)) data;
} pointer_chain_t;

/* Union with struct */
union GTY((desc("%d"))) mixed_union {
    struct {
        int x;
        int y;
    } GTY((tag("point"))) point;
    struct {
        char *GTY((length("strlen(%h.name) + 1"))) name;
        int age;
    } GTY((tag("person"))) person;
};

/* Function pointer array */
typedef int (*GTY((user)) compare_func)(const void*, const void*);
compare_func GTY((length("3"))) compare_funcs[3];

/* Template-like pattern */
#define DECLARE_GTY_STRUCT(name) \
    struct GTY((tag(#name))) name { \
        int id; \
        struct name *GTY((skip)) next; \
    }

DECLARE_GTY_STRUCT(generic_struct);

/* Multiple inheritance-like pattern */
struct GTY(()) base {
    int base_field;
};

struct GTY((tag("derived"))) derived {
    struct base GTY((skip)) parent;
    int derived_field;
};

#endif /* TEST_GTY_H */

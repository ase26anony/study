/* test_state_gty.h - Comprehensive GTY annotations for gengtype state coverage */

#ifndef TEST_STATE_GTY_H
#define TEST_STATE_GTY_H

/* Define GTY macro if not already defined */
#ifndef GTY
#define GTY(x) 
#endif

/* Dummy definitions for GCC internal types */
typedef int tree;
typedef void* rtx;
typedef int gimple;

/* ==================== TYPE_UNDEFINED ==================== */
/* Forward declaration without definition */
struct GTY(()) my_undefined_struct;

/* ==================== TYPE_STRUCT ==================== */
/* Simple struct with tag */
struct GTY((tag("my_struct"))) my_struct {
    int field1;
    char* GTY((skip)) field2;
    tree field3;  /* Using dummy GCC type */
};

/* Another struct for nesting */
struct GTY((tag("nested_struct"))) nested_struct {
    int id;
    struct my_struct* GTY((skip)) ptr;
};

/* ==================== TYPE_USER_STRUCT ==================== */
/* Typedef with user marker */
typedef struct my_struct GTY((user)) my_user_struct_t;

/* Another user struct example */
struct GTY((tag("base_struct"))) base_struct {
    int value;
};

typedef struct base_struct GTY((user)) base_user_t;

/* ==================== TYPE_UNION ==================== */
/* Union with desc tag */
union GTY((desc("0"))) my_union {
    int a;
    char* GTY((skip)) b;
    double c;
    struct my_struct* GTY((skip)) d;
};

/* Union with nested struct */
union GTY((desc("1"))) complex_union {
    int int_val;
    struct {
        int x;
        int y;
    } GTY((tag("point"))) point_val;
    struct my_struct* GTY((skip)) struct_ptr;
};

/* ==================== TYPE_POINTER ==================== */
/* Various pointer declarations */
struct my_struct* GTY((skip)) my_pointer;
union my_union* GTY((skip)) union_pointer;
tree* GTY((skip)) tree_pointer;

/* Pointer in a struct */
struct GTY((tag("ptr_container"))) ptr_container {
    struct my_struct* GTY((skip)) data;
    int count;
};

/* ==================== TYPE_ARRAY ==================== */
/* Fixed-size array */
int GTY((length("10"))) my_fixed_array[10];

/* Array in struct */
struct GTY((tag("array_struct"))) array_struct {
    int GTY((length("count"))) dynamic_array[5];
    int count;
    char GTY((length("strlen(name)+1"))) name[50];
};

/* Array of pointers */
struct my_struct* GTY((length("array_len"))) ptr_array[20];

/* ==================== TYPE_LANG_STRUCT ==================== */
/* Language-specific struct */
struct GTY((special("lang_struct"))) my_lang_struct {
    int lang_specific;
    union {
        int a;
        void* p;
        tree t;
    } u;
    rtx insn;  /* Using dummy GCC type */
};

/* Another lang struct variant */
struct GTY((special("tree_common"))) tree_common_struct {
    tree chain;
    tree type;
    int code;
};

/* ==================== TYPE_SCALAR ==================== */
/* Scalar typedefs with user marker */
typedef int GTY((user)) my_scalar_t;
typedef unsigned long GTY((user)) my_ulong_t;
typedef double GTY((user)) my_double_t;

/* Scalar in struct */
struct GTY((tag("scalar_container"))) scalar_container {
    my_scalar_t value;
    my_double_t precision;
};

/* ==================== TYPE_STRING ==================== */
/* String pointer with length */
const char* GTY((length("strlen(my_string)"))) my_string;

/* String in struct */
struct GTY((tag("string_struct"))) string_struct {
    const char* GTY((length("len"))) data;
    int len;
    char* GTY((length("name_len"))) name;
    int name_len;
};

/* ==================== TYPE_CALLBACK ==================== */
/* Function pointer typedefs */
typedef void (*GTY((user)) simple_callback)(int);
typedef int (*GTY((user)) process_callback)(struct my_struct*, void*);
typedef tree (*GTY((user)) tree_callback)(tree, tree);

/* Callback in struct */
struct GTY((tag("callback_container"))) callback_container {
    simple_callback cb1;
    process_callback cb2;
    void* GTY((skip)) user_data;
};

/* ==================== COMPLEX NESTED EXAMPLE ==================== */
/* Demonstrating complex type relationships */
struct GTY((tag("master_struct"))) master_struct {
    /* Various field types */
    my_scalar_t id;                     /* TYPE_SCALAR */
    struct my_struct* GTY((skip)) data; /* TYPE_POINTER */
    union my_union variant;             /* TYPE_UNION */
    int GTY((length("item_count"))) items[100]; /* TYPE_ARRAY */
    const char* GTY((length("desc_len"))) description; /* TYPE_STRING */
    simple_callback handler;            /* TYPE_CALLBACK */
    struct my_lang_struct lang_data;    /* TYPE_LANG_STRUCT */
    
    /* Nested struct */
    struct {
        int x;
        int y;
        struct nested_struct* GTY((skip)) nested;
    } GTY((tag("coord"))) position;
};

/* Global variables with various types */
extern struct my_struct GTY((tag("global_struct"))) global_var;
extern union my_union GTY((desc("2"))) global_union;
extern my_scalar_t global_scalar;
extern const char* GTY((length("global_len"))) global_string;
extern simple_callback global_callback;

#endif /* TEST_STATE_GTY_H */

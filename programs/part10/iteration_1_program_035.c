/* test_state_gty.h - Comprehensive GTY-annotated types for gengtype state coverage */

#ifndef TEST_STATE_GTY_H
#define TEST_STATE_GTY_H

/* Define GTY macro if not already defined (as in standalone gengtype test) */
#ifndef GTY
#define GTY(x) 
#endif

/* Dummy definitions for GCC internal types to avoid dependency on full GCC tree */
typedef int tree;
typedef void* rtx;
typedef void* gimple;

/* ==================== TYPE_UNDEFINED ==================== */
/* Forward declaration without definition creates TYPE_UNDEFINED */
struct GTY(()) my_undefined_struct;

/* ==================== TYPE_STRUCT ==================== */
/* Regular struct with tag attribute */
struct GTY((tag("my_struct_tag"))) my_struct {
    int field1;
    char* GTY((skip)) field2;  /* skip attribute for pointer field */
    tree field3;               /* GCC internal type */
};

/* ==================== TYPE_USER_STRUCT ==================== */
/* User-defined struct type */
typedef struct my_struct GTY((user)) my_user_struct_t;

/* Another user struct example */
struct GTY((tag("inner_struct"))) inner_struct {
    int data;
};

typedef struct inner_struct GTY((user)) inner_user_t;

/* ==================== TYPE_UNION ==================== */
/* Union with desc attribute for discriminant */
union GTY((desc("0"))) my_union {
    int GTY((tag("0"))) a;      /* integer when discriminant is 0 */
    char* GTY((tag("1"), skip)) b; /* pointer when discriminant is 1 */
    struct my_struct* GTY((tag("2"))) c; /* struct pointer when 2 */
};

/* Union with nested anonymous struct */
union GTY((desc("$type"))) complex_union {
    int type;
    struct GTY((tag("1"))) {
        int x;
        int y;
    } point;
    struct GTY((tag("2"))) {
        char* GTY((skip)) name;
        int id;
    } named;
};

/* ==================== TYPE_POINTER ==================== */
/* Various pointer types */
struct my_struct* GTY((skip)) my_struct_pointer;

/* Chain of pointers */
struct GTY((tag("node"))) list_node {
    int value;
    struct list_node* GTY((skip)) next;
};

/* Pointer in typedef */
typedef struct list_node* GTY((skip)) list_node_ptr;

/* ==================== TYPE_ARRAY ==================== */
/* Fixed-size array */
int GTY((length("10"))) fixed_array[10];

/* Variable-length array in struct */
struct GTY((tag("array_container"))) array_container {
    int count;
    int GTY((length("count"))) variable_array[1]; /* flexible array member */
};

/* Pointer to array */
typedef int (*GTY((skip)) array_ptr)[10];

/* ==================== TYPE_LANG_STRUCT ==================== */
/* Language-specific struct - often used in GCC frontends */
struct GTY((special("lang_struct"))) my_lang_struct {
    int lang_specific_field;
    union {
        tree t;
        rtx r;
    } GTY((desc("0"))) u;
    void* GTY((skip)) extra;
};

/* Another lang struct pattern */
struct GTY((tag("gcc_lang_struct"))) gcc_lang_struct {
    int code;
    union GTY((desc("code"))) {
        struct my_struct* GTY((tag("1"))) s;
        char* GTY((tag("2"), skip)) str;
    } data;
};

/* ==================== TYPE_SCALAR ==================== */
/* Scalar typedef with user attribute */
typedef int GTY((user)) my_scalar_t;

typedef unsigned long GTY((user)) my_ulong_t;

/* Enum type */
enum GTY((user)) my_enum {
    ENUM_VAL1,
    ENUM_VAL2,
    ENUM_VAL3
};

/* ==================== TYPE_STRING ==================== */
/* String pointer with length attribute */
const char* GTY((length("strlen($)"))) my_string;

/* Array of strings */
const char* GTY((length("count"))) string_array[5];

/* String in struct */
struct GTY((tag("string_holder"))) string_holder {
    int len;
    const char* GTY((length("len"))) str;
};

/* ==================== TYPE_CALLBACK ==================== */
/* Function pointer typedef with user attribute */
typedef void (*GTY((user)) my_callback_fn)(int, char*);

/* Callback in struct */
struct GTY((tag("callback_container"))) callback_container {
    my_callback_fn GTY((skip)) callback;
    void* GTY((skip)) user_data;
};

/* Multiple callback types */
typedef int (*GTY((user)) compare_fn)(const void*, const void*);
typedef void (*GTY((user)) cleanup_fn)(void*);

/* ==================== COMPLEX NESTED EXAMPLE ==================== */
/* Demonstrating complex type relationships */
struct GTY((tag("outer"))) outer_struct {
    /* Nested union */
    union GTY((desc("type"))) {
        int GTY((tag("0"))) i;
        struct my_struct* GTY((tag("1"))) s;
        char* GTY((tag("2"), skip)) str;
    } data;
    
    /* Array of pointers */
    struct my_struct* GTY((length("count"), skip)) ptr_array[5];
    
    /* Callback */
    my_callback_fn GTY((skip)) handler;
    
    /* String */
    const char* GTY((length("strlen($)"))) description;
    
    /* Nested struct */
    struct GTY((tag("inner"))) {
        int id;
        my_scalar_t value;
    } inner;
};

/* Global variables with various types for gengtype to process */
extern struct my_struct GTY((skip)) global_struct;
extern union my_union GTY((skip)) global_union;
extern struct my_lang_struct GTY((skip)) global_lang_struct;
extern my_callback_fn GTY((skip)) global_callback;

#endif /* TEST_STATE_GTY_H */

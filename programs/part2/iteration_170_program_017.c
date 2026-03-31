/* test_gengtype_types.h - Comprehensive type definitions for gengtype coverage */
#ifndef TEST_GENGTYPE_TYPES_H
#define TEST_GENGTYPE_TYPES_H

/* Include gtype-desc.h for GTY macro if not defined */
#ifndef GTY
#define GTY(x) x
#endif

/* TYPE_SCALAR: Basic typedefs */
typedef int my_int;
typedef unsigned int my_uint;
typedef char my_char;
typedef double my_double;
typedef long long my_longlong;

/* TYPE_STRING: String type definitions */
typedef const char *string_t;
typedef char *mutable_string_t;

/* TYPE_STRUCT: Plain C structs (not GTY-tagged) */
struct plain_s {
    int a;
    double b;
};

struct another_plain {
    char c;
    float f;
};

/* TYPE_USER_STRUCT: GTY-tagged structs */
struct GTY(()) user_s {
    struct plain_s *p;  /* Pointer to plain struct */
    my_int count;
};

struct GTY(()) tree_node {
    int code;
    struct tree_node *GTY((skip)) left;
    struct tree_node *GTY((skip)) right;
    string_t name;
};

/* TYPE_UNION: Union definitions */
union my_u {
    int i;
    void *p;
    double d;
};

union GTY(()) tagged_union {
    int tag;
    struct user_s *GTY((tag("0"))) usr;
    string_t GTY((tag("1"))) str;
};

/* TYPE_POINTER: Various pointer types */
typedef struct user_s *user_ptr_t;
typedef void *generic_ptr_t;
typedef int *int_ptr_t;

/* TYPE_ARRAY: Array types */
struct GTY(()) array_container {
    int fixed_array[10];
    struct user_s *GTY((length("count"))) variable_array[1];
    my_int count;
};

/* TYPE_CALLBACK: Function pointer types */
typedef void (*callback_fn)(int);
typedef int (*compare_fn)(const void *, const void *);
typedef struct user_s *(*allocator_fn)(void);

struct GTY(()) callback_container {
    callback_fn handler;
    compare_fn comparator;
    allocator_fn allocator;
};

/* Complex nested types to ensure deep traversal */
struct GTY(()) outer_struct {
    struct GTY(()) inner_struct {
        int value;
        struct outer_struct *parent;
    } inner;
    
    union GTY(()) inner_union {
        int int_val;
        struct inner_struct *struct_ptr;
    } u;
    
    struct array_container arrays;
    callback_fn callbacks[5];
};

/* Recursive type pattern */
struct GTY(()) recursive_node {
    int data;
    struct recursive_node *GTY((skip)) next;
    struct recursive_node *GTY((skip)) prev;
};

/* Language-specific struct for TYPE_LANG_STRUCT */
#ifdef GENERATOR_FILE
struct GTY(()) lang_specific_struct {
    int generator_only_field;
    struct user_s *data;
};
#endif

/* Conditional compilation for different contexts */
#if defined(GCC) || defined(GENERATOR_FILE)
struct GTY(()) context_specific {
    int context_id;
    #ifdef GENERATOR_FILE
    string_t generator_data;
    #endif
    #ifdef GCC
    void *gcc_data;
    #endif
};
#endif

/* Mixed pointer types in arrays */
struct GTY(()) mixed_pointers {
    void *void_ptr;
    int *int_ptr;
    struct user_s **user_ptr_ptr;
    callback_fn *callback_array[3];
};

/* Union containing GTY-tagged pointer */
union GTY(()) union_with_gty {
    struct user_s *GTY((tag("0"))) user_ptr;
    struct array_container *GTY((tag("1"))) array_ptr;
    callback_fn GTY((tag("2"))) callback;
};

/* Struct with nested anonymous struct/union */
struct GTY(()) anonymous_member {
    struct {
        int x;
        int y;
    } point;
    
    union {
        int i;
        double d;
    } value;
    
    struct GTY(()) {
        int id;
        string_t name;
    } info;
};

/* Template-like pattern using macros */
#define DECLARE_GTY_STRUCT(name, field_type) \
    struct GTY(()) name { \
        field_type data; \
        struct name *next; \
    }

DECLARE_GTY_STRUCT(int_list, int);
DECLARE_GTY_STRUCT(string_list, string_t);

/* Undefined type forward declaration (for TYPE_UNDEFINED) */
struct undefined_struct;
typedef struct undefined_struct *undefined_ptr_t;

#endif /* TEST_GENGTYPE_TYPES_H */

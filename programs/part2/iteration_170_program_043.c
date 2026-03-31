/* test_gengtype_types.h - Comprehensive type definitions for gengtype coverage */

#ifndef TEST_GENGTYPE_TYPES_H
#define TEST_GENGTYPE_TYPES_H

/* Include gtype-desc.h for GTY macro if not already defined */
#ifndef GTY
#define GTY(x) x
#endif

/* ========== TYPE_SCALAR ========== */
typedef int my_int;
typedef unsigned long my_ulong;
typedef double my_double;
typedef char my_char;
typedef _Bool my_bool;

/* ========== TYPE_STRING ========== */
typedef const char *string_t;
typedef char *mutable_string_t;

/* ========== TYPE_CALLBACK ========== */
typedef void (*callback_fn)(int);
typedef int (*comparator_fn)(const void *, const void *);
typedef void (*void_fn_ptr)(void);

/* ========== TYPE_STRUCT (untagged) ========== */
struct plain_s {
    int a;
    double b;
};

struct another_plain {
    char c;
    long l;
};

/* ========== TYPE_UNION ========== */
union my_u {
    int i;
    void *p;
    double d;
};

union tagged_union {
    int tag;
    struct {
        int x, y;
    } point;
    char str[16];
};

/* ========== TYPE_ARRAY ========== */
/* Arrays will be defined within structs */

/* ========== TYPE_USER_STRUCT (GTY-tagged) ========== */
struct GTY(()) user_s {
    struct plain_s *p;  /* TYPE_POINTER to TYPE_STRUCT */
    int count;
};

struct GTY(()) tree_node {
    int code;
    struct tree_node *GTY((skip)) left;
    struct tree_node *right;
    union my_u value;
};

struct GTY(()) complex_struct {
    /* Multiple pointer types */
    struct user_s *GTY((tag("0"))) user_ptr;
    struct complex_struct *next;  /* Recursive pointer */
    
    /* Array types */
    int arr[10];  /* TYPE_ARRAY */
    struct plain_s *ptr_array[5];  /* Array of pointers */
    
    /* String type */
    const char *GTY((length("strlen(%h.name) + 1"))) name;
    
    /* Callback type */
    callback_fn handler;
    
    /* Scalar types */
    my_int id;
    my_double weight;
    
    /* Union type */
    union tagged_union data;
};

/* Another GTY-tagged struct with nested structures */
struct GTY(()) outer_struct {
    struct complex_struct inner;
    struct outer_struct *GTY((skip)) sibling;
    void_fn_ptr cleanup;
};

/* ========== TYPE_POINTER ========== */
/* Additional pointer typedefs */
typedef struct user_s *user_ptr_t;
typedef struct complex_struct **double_ptr_t;

/* ========== TYPE_LANG_STRUCT ========== */
/* Language-specific structs using conditional compilation */
#ifdef GENERATOR_FILE
struct GTY(()) lang_struct_gen {
    int generator_specific;
    void *gen_data;
};
#else
struct GTY(()) lang_struct_normal {
    int normal_field;
    struct lang_struct_gen *GTY((skip)) gen_ptr;
};
#endif

/* Conditional for backend-specific structures */
#ifdef GCC_INSN_CODES_H
struct GTY(()) insn_pattern {
    const char *pattern;
    int opcode;
};
#endif

/* ========== More Complex Patterns ========== */

/* Struct containing array of callbacks */
struct GTY(()) callback_container {
    callback_fn handlers[8];  /* Array of function pointers */
    int active_count;
};

/* Union containing GTY-tagged pointer */
union GTY(()) gty_union {
    struct user_s *GTY((tag("1"))) userp;
    struct complex_struct *cptr;
    long as_long;
};

/* Self-referential structure with multiple pointer types */
struct GTY(()) linked_item {
    int value;
    struct linked_item *next;
    struct linked_item **prev_ptr;  /* Pointer to pointer */
    struct linked_item *neighbors[4];  /* Array of pointers */
};

/* Struct with variable-length array annotation */
struct GTY(()) var_len_struct {
    int length;
    int data[1];  /* Variable length array */
};

/* Typedef for a GTY-tagged union */
typedef union gty_union GTY(()) gty_union_t;

#endif /* TEST_GENGTYPE_TYPES_H */

/* test_gengtype_types.h - Comprehensive type definitions for gengtype coverage */

#ifndef TEST_GENGTYPE_TYPES_H
#define TEST_GENGTYPE_TYPES_H

/* Include gtype-desc.h if available in GCC build context */
#ifdef HAVE_GTYPE_DESC_H
#include "gtype-desc.h"
#endif

/* For language-specific structs */
#ifdef GENERATOR_FILE
#define LANG_STRUCT_MARKER
#else
#define LANG_STRUCT_MARKER
#endif

/* ========== TYPE_SCALAR ========== */
typedef int my_int;
typedef unsigned int my_uint;
typedef char my_char;
typedef double my_double;
typedef long long my_longlong;

/* ========== TYPE_STRING ========== */
typedef const char *string_t;
typedef char *mutable_string_t;

/* ========== TYPE_STRUCT (plain, untagged) ========== */
struct plain_s {
    int a;
    double b;
};

struct another_plain {
    struct plain_s *link;
    int count;
};

/* ========== TYPE_USER_STRUCT (GTY-tagged) ========== */
struct GTY(()) user_s {
    struct plain_s *p;  /* TYPE_POINTER to plain struct */
    my_int value;       /* TYPE_SCALAR */
};

struct GTY(()) complex_user_s {
    struct user_s *next;  /* Recursive pointer */
    struct user_s *prev;
    string_t name;        /* TYPE_STRING */
    int flags;
};

/* ========== TYPE_UNION ========== */
union my_u {
    int i;
    void *p;
    double d;
};

union GTY(()) tagged_union {
    struct user_s *us;
    struct complex_user_s *cus;
    int tag;
};

/* ========== TYPE_ARRAY ========== */
struct GTY(()) array_container {
    int fixed_array[10];           /* Fixed-size array */
    struct user_s *ptr_array[5];   /* Array of pointers */
    int multi_dim[3][4];           /* Multi-dimensional */
};

/* ========== TYPE_CALLBACK ========== */
typedef void (*callback_fn)(int, void*);
typedef int (*compare_fn)(const void*, const void*);

struct GTY(()) callback_container {
    callback_fn handler;           /* Function pointer field */
    compare_fn comparator;
    void *user_data;
};

/* ========== TYPE_LANG_STRUCT ========== */
/* Language-specific struct - only processed in generator context */
#ifdef GENERATOR_FILE
struct LANG_STRUCT_MARKER lang_specific_s {
    int generator_only_field;
    void *generator_data;
};
#endif

/* ========== Complex nested example ========== */
struct GTY(()) tree_node;

struct GTY(()) tree_common {
    struct tree_node *chain;
    struct tree_node *type;
    int code;
    unsigned int side_effects_flag:1;
    unsigned int constant_flag:1;
    unsigned int addressable_flag:1;
    unsigned int volatile_flag:1;
    unsigned int readonly_flag:1;
    unsigned int unsigned_flag:1;
    unsigned int asm_written_flag:1;
    unsigned int nowarning_flag:1;
    unsigned int used_flag:1;
    unsigned int nothrow_flag:1;
    unsigned int static_flag:1;
    unsigned int public_flag:1;
    unsigned int private_flag:1;
    unsigned int protected_flag:1;
    unsigned int deprecated_flag:1;
    unsigned int invariant_flag:1;
};

union GTY((desc ("TREE_CODE ((tree) &%h)"), 
           chain_next ("CODE_CONTAINS_STRUCT (TREE_CODE ((tree) &%h), TS_COMMON) ? ((union tree_node *) ((tree) &%h)->common.chain) : NULL"))) tree_node {
    struct tree_common common;
};

struct GTY(()) tree_decl_minimal {
    struct tree_common common;
    string_t filename;
    unsigned int linenum;
};

/* ========== More pointer types ========== */
typedef struct user_s *user_ptr_t;
typedef struct complex_user_s **double_ptr_t;

/* ========== Mixed container with all types ========== */
struct GTY(()) mega_container {
    /* Scalar */
    my_int id;
    my_double weight;
    
    /* String */
    string_t description;
    
    /* Struct */
    struct plain_s plain;
    
    /* User struct */
    struct user_s *user;
    
    /* Union */
    union my_u variant;
    
    /* Pointer */
    void *generic_ptr;
    
    /* Array */
    int scores[20];
    struct user_s *users[10];
    
    /* Callback */
    callback_fn on_event;
    
    /* Language struct pointer */
#ifdef GENERATOR_FILE
    struct lang_specific_s *lang_data;
#endif
    
    /* Self-reference for recursion */
    struct mega_container *next;
};

/* ========== Undefined type forward declaration ========== */
struct undefined_struct;  /* This should trigger TYPE_UNDEFINED */

#endif /* TEST_GENGTYPE_TYPES_H */

/* Test header for gengtype-state.cc coverage testing */
#ifndef GTY_TEST_H
#define GTY_TEST_H

#include "gtype-desc.h"

/* TYPE_SCALAR: Basic scalar types */
typedef int scalar_int_t;
typedef double scalar_double_t;
typedef char scalar_char_t;

/* Enum type (also scalar) */
enum color { RED, GREEN, BLUE };

/* TYPE_STRING: String structure */
struct GTY(()) gcc_string {
    int length;
    char *GTY((skip)) data;  /* Skip data pointer for GC */
};

/* TYPE_CALLBACK: Function pointer type */
typedef int (*callback_t)(void *user_data, int value);

/* TYPE_ARRAY: Array typedef */
typedef int vec4_t[4];
typedef struct gcc_string* string_array_t[10];

/* Non-annotated struct (may become TYPE_UNDEFINED) */
struct unannotated_struct {
    int x;
    double y;
};

/* TYPE_STRUCT: Basic annotated struct */
struct GTY(()) basic_struct {
    int id;
    double value;
    char name[32];
    vec4_t coordinates;  /* TYPE_ARRAY member */
};

/* TYPE_USER_STRUCT: User-defined struct with special handling */
struct GTY((user)) user_struct {
    void *GTY((skip)) opaque_data;
    int user_id;
    callback_t handler;  /* TYPE_CALLBACK member */
};

/* TYPE_UNION: Union type */
union GTY(()) data_union {
    int int_val;
    double double_val;
    char *GTY((skip)) string_val;
    struct basic_struct *GTY((tag("0"))) struct_ptr;
};

/* TYPE_POINTER: Pointer types */
typedef struct basic_struct* basic_ptr_t;
typedef union data_union* union_ptr_t;
typedef callback_t* callback_ptr_t;

/* Linked list structure for traversal */
struct GTY(()) list_node {
    int data;
    struct list_node *next;
    struct list_node *prev;
};

/* Complex nested structure */
struct GTY(()) container {
    /* TYPE_STRUCT members */
    struct basic_struct basic;
    
    /* TYPE_UNION member */
    union data_union variant;
    
    /* TYPE_POINTER members */
    struct user_struct *user;
    struct list_node *head;
    struct gcc_string *title;
    
    /* TYPE_ARRAY members */
    basic_ptr_t ptr_array[5];
    string_array_t strings;
    
    /* TYPE_CALLBACK member */
    callback_t notify;
    
    /* TYPE_SCALAR members */
    scalar_int_t count;
    scalar_double_t total;
    enum color bg_color;
    
    /* Reference to non-annotated type */
    struct unannotated_struct *unannotated;
};

/* TYPE_LANG_STRUCT: Language-specific structure */
#ifdef LANGUAGE_HOOKS
struct GTY(()) lang_struct {
    int lang_specific;
    void *GTY((skip)) lang_data;
};
#endif

/* Root structure containing everything */
struct GTY(()) root_container {
    struct container main_container;
    struct user_struct *users[3];
    union data_union variants[2];
    struct list_node *node_list;
    struct gcc_string *description;
    
    /* Array of different pointer types */
    void *GTY((skip)) mixed_ptrs[4];
};

/* External declaration to force inclusion */
extern struct root_container * GTY((root)) global_root;

#endif /* GTY_TEST_H */

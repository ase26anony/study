/* Test header for gengtype-state.cc coverage */
#ifndef GTY_TEST_H
#define GTY_TEST_H

#include "gtype-desc.h"

/* TYPE_SCALAR: Basic scalar types */
typedef int scalar_int_t;
typedef double scalar_double_t;

/* TYPE_ENUM (processed as scalar) */
enum color { RED, GREEN, BLUE };

/* TYPE_UNDEFINED: Type without GTY marker but referenced by annotated types */
struct undefined_struct {
    int x;
    double y;
};

/* TYPE_STRUCT: Basic struct with GTY marker */
struct GTY(()) basic_struct {
    int id;
    char name[32];
    double value;
};

/* TYPE_STRUCT with nested references */
struct GTY(()) complex_struct {
    struct basic_struct *GTY((tag("0"))) base;
    struct undefined_struct *GTY((skip)) undef_ref;  /* References undefined type */
    int GTY((skip)) data[10];  /* TYPE_ARRAY inside struct */
};

/* TYPE_USER_STRUCT: Struct with user marker */
struct GTY((user)) user_struct {
    void *GTY((skip)) user_data;
    int user_id;
};

/* TYPE_UNION: Union type */
union GTY(()) data_union {
    int int_val;
    double double_val;
    char *GTY((tag("1"))) string_val;
    struct basic_struct *GTY((tag("2"))) struct_val;
};

/* TYPE_POINTER: Various pointer types */
typedef struct basic_struct *basic_ptr_t;
typedef int *int_ptr_t;
typedef void *void_ptr_t;

/* TYPE_ARRAY: Array typedefs */
typedef int int_array_10[10];
typedef struct basic_struct *struct_ptr_array[5];

/* TYPE_STRING: String-like structure */
struct GTY(()) gcc_string {
    int GTY((skip)) length;
    char *GTY((tag("3"))) data;
};

/* TYPE_CALLBACK: Function pointer types */
typedef int (*callback_func)(void *GTY((skip)) data, int param);
typedef void (*simple_callback)(void);

/* TYPE_CALLBACK in struct */
struct GTY(()) callback_container {
    callback_func GTY((skip)) handler;
    void *GTY((skip)) context;
    simple_callback GTY((skip)) cleanup;
};

/* Linked list for TYPE_POINTER chain */
struct GTY(()) list_node {
    int GTY((skip)) value;
    struct list_node *GTY((tag("4"))) next;
    struct list_node *GTY((tag("5"))) prev;
};

/* TYPE_ARRAY of pointers in struct */
struct GTY(()) array_container {
    struct basic_struct *GTY((tag("6"))) items[8];
    union data_union GTY((skip)) variants[4];
    callback_func GTY((skip)) handlers[3];
};

/* Mixed type container */
struct GTY(()) mixed_container {
    /* TYPE_SCALAR members */
    int counter;
    double ratio;
    enum color color;
    
    /* TYPE_POINTER members */
    struct basic_struct *GTY((tag("7"))) base;
    struct gcc_string *GTY((tag("8"))) str;
    
    /* TYPE_UNION member */
    union data_union GTY((skip)) data;
    
    /* TYPE_ARRAY members */
    int scores[5];
    struct list_node *GTY((tag("9"))) nodes[3];
    
    /* TYPE_CALLBACK member */
    callback_func GTY((skip)) notify;
    
    /* Reference to TYPE_USER_STRUCT */
    struct user_struct *GTY((tag("10"))) user;
};

/* Root structure containing all types - ensures gengtype traverses everything */
struct GTY(()) root_container {
    struct basic_struct GTY((skip)) basic;
    struct complex_struct *GTY((tag("11"))) complex;
    union data_union GTY((skip)) union_data;
    struct gcc_string *GTY((tag("12"))) strings[4];
    struct list_node *GTY((tag("13"))) list_head;
    struct array_container GTY((skip)) arrays;
    struct mixed_container *GTY((tag("14"))) mixed;
    struct callback_container GTY((skip)) callbacks;
    struct user_struct *GTY((tag("15"))) users[2];
    
    /* Various pointer types */
    int_ptr_t GTY((skip)) int_ptrs[5];
    void_ptr_t GTY((skip)) void_ptrs[3];
    
    /* Direct scalar arrays */
    scalar_int_t GTY((skip)) scalar_ints[10];
    scalar_double_t GTY((skip)) scalar_doubles[5];
};

/* Additional TYPE_LANG_STRUCT simulation */
/* In GCC, these are language-specific structures marked with GTY(()) */
struct GTY(()) lang_struct_base {
    int lang_specific;
};

struct GTY(()) c_lang_struct {
    struct lang_struct_base base;
    int c_specific_field;
};

struct GTY(++) cpp_lang_struct {
    struct lang_struct_base base;
    void *GTY((skip)) cpp_specific;
};

/* Chain of structures to ensure deep traversal */
struct GTY(()) chain_link {
    int id;
    struct chain_link *GTY((tag("16"))) next;
    struct chain_link *GTY((tag("17"))) child;
    struct chain_link *GTY((tag("18"))) siblings[2];
};

/* Self-referential structure */
struct GTY(()) self_ref {
    int data;
    struct self_ref *GTY((tag("19"))) self;
    struct self_ref *GTY((tag("20"))) mirror;
};

#endif /* GTY_TEST_H */

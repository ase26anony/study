/* test-gtype-base.h - Base type definitions for gengtype coverage testing */

#ifndef TEST_GTYPE_BASE_H
#define TEST_GTYPE_BASE_H

#include "ansidecl.h"

/* Forward declarations to potentially trigger TYPE_UNDEFINED */
struct forward_declared_struct;
typedef struct forward_declared_struct *forward_ptr_t;

/* TYPE_SCALAR examples */
typedef int my_scalar_t;
typedef unsigned long my_ulong_t;
typedef enum { RED, GREEN, BLUE } color_enum;

/* TYPE_STRING */
typedef const char *string_type;

/* TYPE_CALLBACK */
typedef void (*callback_func_t)(void *data, int value);
typedef int (*compare_func_t)(const void *, const void *);

/* TYPE_POINTER */
typedef struct my_struct *struct_ptr_t;
typedef union my_union *union_ptr_t;

/* TYPE_ARRAY - variable length */
struct var_array_struct {
    int length;
    int *data GTY((length("%0.length")));
};

/* TYPE_ARRAY - fixed length */
struct fixed_array_struct {
    int data[10];
    struct my_struct *ptrs[5];
};

/* TYPE_STRUCT with various fields */
struct my_struct GTY(()) {
    int id;                         /* scalar field */
    string_type name;               /* string field */
    struct my_struct *next GTY((skip)); /* pointer with skip */
    struct my_struct *prev GTY((chain_prev("%0.next"))); /* chain pointer */
    union my_union *variant;        /* pointer to union */
    callback_func_t callback;       /* callback field */
    struct var_array_struct array;  /* nested struct with array */
    forward_ptr_t forward_ref;      /* forward reference */
};

/* TYPE_UNION with discriminator */
union my_union GTY(()) {
    int int_value;
    string_type str_value;
    struct my_struct *struct_ptr;
    double double_value;
};

/* Discriminated union */
struct tagged_union GTY((desc("tag"))) {
    enum { TAG_INT, TAG_STR, TAG_PTR } tag;
    union {
        int int_val;
        const char *str_val;
        void *ptr_val;
    } u GTY((desc("%0.tag")));
};

/* Linked list structure */
struct list_node GTY(()) {
    int value;
    struct list_node *next GTY((chain_next("%0.next")));
    struct list_node *prev GTY((chain_prev("%0.next")));
};

#endif /* TEST_GTYPE_BASE_H */

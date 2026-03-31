/* test-basic-structs.h - Cover TYPE_STRUCT, TYPE_SCALAR, TYPE_ARRAY, TYPE_POINTER */

#ifndef TEST_BASIC_STRUCTS_H
#define TEST_BASIC_STRUCTS_H

#include "config.h"
#include "system.h"

/* TYPE_SCALAR: Basic scalar types */
typedef enum color {
    RED,
    GREEN,
    BLUE
} color GTY(());

/* TYPE_STRUCT: Basic structure with scalar fields */
struct basic_struct GTY(()) {
    int int_field;          /* TYPE_SCALAR */
    long long_field;        /* TYPE_SCALAR */
    double double_field;    /* TYPE_SCALAR */
    float float_field;      /* TYPE_SCALAR */
    char char_field;        /* TYPE_SCALAR */
    bool bool_field;        /* TYPE_SCALAR (C++) */
    color enum_field;       /* TYPE_SCALAR via enum */
};

/* TYPE_ARRAY: Arrays within structures */
struct array_struct GTY(()) {
    int fixed_array[10];                    /* TYPE_ARRAY fixed size */
    struct basic_struct struct_array[5];    /* TYPE_ARRAY of structs */
    char* string_array[3];                  /* TYPE_ARRAY of TYPE_STRING */
};

/* TYPE_POINTER: Various pointer types */
struct pointer_struct GTY(()) {
    struct basic_struct* direct_ptr;        /* TYPE_POINTER to struct */
    struct basic_struct** double_ptr;       /* TYPE_POINTER to pointer */
    void* void_ptr;                         /* TYPE_POINTER with TYPE_UNDEFINED */
    int (*func_ptr)(int, int);              /* TYPE_POINTER to function */
};

/* Chain of structures for recursive traversal */
struct linked_node GTY(()) {
    int data;
    struct linked_node* next;  /* TYPE_POINTER forming linked list */
    struct linked_node* prev;  /* TYPE_POINTER forming doubly linked list */
};

/* Global variables to ensure processing */
extern struct basic_struct global_basic GTY(());
extern struct array_struct global_array GTY(());
extern struct pointer_struct global_pointers GTY(());
extern struct linked_node* global_list GTY(());

#endif /* TEST_BASIC_STRUCTS_H */

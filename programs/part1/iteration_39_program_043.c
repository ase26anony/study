/* Test header for gengtype coverage - covers all type categories in statistics collection */

#ifndef MYTEST_H
#define MYTEST_H

#include "config.h"
#include "system.h"
#include "coretypes.h"

/* Forward declarations for struct/union types */
struct my_test_struct;
union my_test_union;

/* TYPE_SCALAR: Basic scalar type with GTY annotation */
typedef int GTY(()) my_scalar_t;

/* TYPE_STRING: String pointer type */
extern const char * GTY(()) my_test_string;

/* TYPE_POINTER: Various pointer types */
struct my_test_struct * GTY(()) my_struct_pointer;
int * GTY(()) my_int_pointer;
void * GTY(()) my_void_pointer;

/* TYPE_ARRAY: Array types */
int GTY(()) my_int_array[10];
struct my_test_struct * GTY(()) my_struct_array[5];

/* TYPE_STRUCT: Structure type */
struct GTY(()) my_test_struct {
    int field1;
    my_scalar_t field2;
    struct my_test_struct * GTY(()) next;
};

/* TYPE_USER_STRUCT: User-defined structure (using typedef) */
typedef struct GTY(()) {
    int user_field1;
    double user_field2;
} my_user_struct_t;

/* TYPE_UNION: Union type */
union GTY(()) my_test_union {
    int int_val;
    double double_val;
    char * GTY(()) string_val;
    struct my_test_struct * GTY(()) struct_ptr;
};

/* TYPE_CALLBACK: Function pointer type */
typedef void (*GTY(()) my_callback_fn)(int, const char*);
extern my_callback_fn GTY(()) current_callback;

/* TYPE_LANG_STRUCT: Language-specific structure */
#ifdef GENERATOR_FILE
struct GTY(()) lang_test_struct {
    int lang_specific;
    void * GTY(()) lang_data;
};
#endif

/* Complex nested example covering multiple types */
struct GTY(()) container_struct {
    /* Scalar */
    int count;
    
    /* String */
    const char * GTY(()) name;
    
    /* Pointer */
    struct container_struct * GTY(()) parent;
    
    /* Array */
    my_scalar_t GTY(()) values[8];
    
    /* Struct */
    my_user_struct_t config;
    
    /* Union */
    union my_test_union data;
    
    /* Callback */
    my_callback_fn GTY(()) notify;
    
    /* Pointer array */
    struct my_test_struct * GTY(()) items[20];
};

/* Template-like structure for additional coverage */
struct GTY((chain_next ("%h.next"), chain_prev ("%h.prev"))) linked_node {
    int id;
    const char * GTY(()) label;
    struct linked_node * GTY(()) next;
    struct linked_node * GTY(()) prev;
};

/* Skip annotation example */
struct GTY((skip)) skipped_struct {
    int internal_data;
    void * GTY(()) managed_ptr;
};

/* Length annotation for arrays */
struct GTY(()) variable_array_container {
    int count;
    int * GTY((length ("%h.count"))) dynamic_array;
};

#endif /* MYTEST_H */

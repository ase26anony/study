#ifndef TEST_CALLBACK_TYPES_H
#define TEST_CALLBACK_TYPES_H

/* Include necessary GCC headers for gengtype parsing */
#include "config.h"
#include "system.h"

/* 
 * TYPE_CALLBACK: Function pointer with GTY((callback)) annotation
 * This is the primary target to trigger the uncovered block
 */
typedef void (*simple_callback_fn)(int) GTY((callback));

/* Another callback type with different signature */
typedef int (*complex_callback_fn)(const char*, void*) GTY((callback));

/* 
 * TYPE_STRUCT: Simple struct with GTY annotation
 * This ensures write_state_struct_type is called
 */
struct GTY(()) simple_struct {
    int field1;
    double field2;
};

/* 
 * TYPE_UNION: Union with GTY annotation
 * This ensures write_state_union_type is called
 */
union GTY(()) my_union {
    int a;
    void* GTY((skip)) b;  /* Skip annotation for void pointer */
    double c;
};

/* 
 * TYPE_POINTER: Typedef pointer with GTY tag
 * This ensures write_state_pointer_type is called
 */
typedef simple_struct* simple_ptr GTY((tag("SIMPLE_PTR")));

/* 
 * TYPE_ARRAY: Struct containing array
 * This ensures write_state_array_type is called
 */
struct GTY(()) with_array {
    int arr[10];
    char name[32];
};

/* 
 * TYPE_SCALAR: Scalar typedef with length annotation
 * This ensures write_state_scalar_type is called
 */
typedef unsigned my_scalar GTY((length));

/* 
 * TYPE_STRING: String pointer type
 * This ensures write_state_string_type is called
 */
typedef const char* my_string GTY((length));

/* 
 * Nested callback structure: Struct containing callback function pointers
 * This tests traversal into nested callback types
 */
struct GTY(()) callback_container {
    /* Array of callback functions */
    simple_callback_fn handlers[2];
    
    /* Single callback function pointer */
    complex_callback_fn processor;
    
    /* Regular data field */
    int id;
};

/* 
 * Union containing callback type
 * This tests callback types within unions
 */
union GTY(()) callback_union {
    simple_callback_fn fn;
    complex_callback_fn complex_fn;
    int callback_id;
};

/* 
 * More complex nested structure with multiple callback types
 */
struct GTY(()) nested_callbacks {
    /* Direct callback field */
    simple_callback_fn direct_cb;
    
    /* Struct containing callback */
    callback_container container;
    
    /* Union with callback */
    callback_union cb_union;
    
    /* Pointer to callback type */
    simple_callback_fn* callback_ptr GTY((tag("CALLBACK_PTR")));
};

/* 
 * Structure with callback array
 */
struct GTY(()) callback_array_struct {
    /* Array of callback function pointers */
    complex_callback_fn callbacks[5];
    
    /* Dynamic array of callbacks */
    simple_callback_fn* dynamic_callbacks GTY((length("dynamic_count")));
    int dynamic_count;
};

/* 
 * User-defined structure (TYPE_USER_STRUCT)
 * This would require additional handling but included for completeness
 */

/* 
 * Structure with multiple callback types mixed with regular data
 */
struct GTY(()) mixed_types {
    /* Callback field */
    simple_callback_fn cb;
    
    /* Regular struct */
    simple_struct data;
    
    /* Union */
    my_union variant;
    
    /* Pointer */
    simple_ptr ptr;
    
    /* Array */
    with_array array_data;
    
    /* Scalar */
    my_scalar length;
    
    /* String */
    my_string name;
};

/* 
 * Self-referential structure with callback
 */
struct GTY(()) recursive_callback {
    simple_callback_fn processor;
    struct recursive_callback* GTY((skip)) next;  /* Skip to avoid infinite recursion */
};

/* 
 * Callback that takes another callback as parameter (through typedef)
 */
typedef void (*higher_order_callback)(simple_callback_fn) GTY((callback));

/* 
 * Structure using higher-order callback
 */
struct GTY(()) higher_order_container {
    higher_order_callback mapper;
    simple_callback_fn target;
};

#endif /* TEST_CALLBACK_TYPES_H */

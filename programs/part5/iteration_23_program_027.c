/* test_types.h - Comprehensive type definitions for gengtype coverage */

#ifndef TEST_TYPES_H
#define TEST_TYPES_H

/* Include GCC's GTY macros */
#ifndef GTY
#define GTY(x) 
#endif

/* Forward declaration for TYPE_UNDEFINED */
struct opaque_struct;

/* TYPE_SCALAR: Fundamental scalar types */
typedef enum color {
    RED,
    GREEN,
    BLUE
} color_enum GTY(());

typedef int scalar_int GTY(());
typedef char scalar_char GTY(());
typedef long scalar_long GTY(());
typedef float scalar_float GTY(());
typedef double scalar_double GTY(());

/* TYPE_STRUCT: Basic struct with various members */
struct GTY(()) basic_struct {
    int id;
    char name[32];
    float value;
    color_enum color;
};

/* TYPE_STRUCT: Nested struct with anonymous struct */
struct GTY(()) complex_struct {
    struct GTY(()) {
        int x;
        int y;
    } point;
    
    struct basic_struct basic;
    
    /* Bit-fields */
    unsigned int flags : 4;
    unsigned int mode : 2;
    
    /* Chain pointers for GTY options */
    struct complex_struct *GTY((skip)) next;
    struct complex_struct *GTY((chain_prev ("%h.next"))) prev;
};

/* TYPE_USER_STRUCT: User-defined struct */
struct GTY((user)) user_struct {
    int user_data;
    void *GTY((skip)) user_ptr;
};

/* TYPE_UNION: Basic union */
union GTY(()) basic_union {
    int int_val;
    float float_val;
    char *GTY((length ("strlen(%h.char_ptr)+1"))) char_ptr;
    double double_val;
};

/* TYPE_UNION: Tagged union within struct */
struct GTY(()) tagged_union_container {
    enum { INT_TYPE, FLOAT_TYPE, STRING_TYPE } tag;
    union GTY((desc ("%0.tag"))) {
        int int_value;
        float float_value;
        char *GTY((length ("strlen(%h.string_value)+1"))) string_value;
    } value;
};

/* TYPE_POINTER: Various pointer types */
typedef struct basic_struct *struct_ptr GTY(());
typedef union basic_union *union_ptr GTY(());
typedef int *int_ptr GTY(());
typedef void *void_ptr GTY(());
typedef struct opaque_struct *opaque_ptr GTY(());

/* Pointer to pointer */
typedef struct_ptr *struct_ptr_ptr GTY(());

/* TYPE_ARRAY: Various array types */
typedef int int_array[10] GTY(());
typedef struct basic_struct struct_array[5] GTY(());
typedef union basic_union union_array[3] GTY(());
typedef int_ptr pointer_array[8] GTY(());

/* Multi-dimensional array */
typedef int matrix[3][3] GTY(());

/* TYPE_STRING: String types */
typedef char *string_ptr GTY((length ("strlen(%h)+1")));
typedef const char *const_string_ptr GTY(());

/* TYPE_CALLBACK: Function pointer types */
typedef void (*simple_callback)(void) GTY(());
typedef int (*complex_callback)(struct basic_struct *, int) GTY(());

/* Struct containing callback */
struct GTY(()) callback_container {
    simple_callback cb1;
    complex_callback cb2;
    void (*GTY((skip)) internal_cb)(void);
};

/* Linked list structure for chain testing */
struct GTY(()) list_node {
    int data;
    struct list_node *GTY((chain_next ("%h.next"))) next;
    struct list_node *GTY((chain_prev ("%h.prev"))) prev;
};

/* Container with length field */
struct GTY(()) variable_array_container {
    size_t count;
    int *GTY((length ("%h.count"))) items;
};

/* Nested type references */
struct GTY(()) master_container {
    /* Reference to all major types */
    struct basic_struct basic;
    union basic_union union_val;
    struct complex_struct complex;
    struct user_struct user;
    
    /* Arrays */
    struct_array structs;
    union_array unions;
    
    /* Pointers */
    struct_ptr ptr_to_struct;
    union_ptr ptr_to_union;
    opaque_ptr ptr_to_opaque;
    
    /* Strings */
    string_ptr dynamic_string;
    const_string_ptr const_string;
    
    /* Callbacks */
    struct callback_container callbacks;
    
    /* Scalar types */
    scalar_int int_val;
    scalar_float float_val;
    scalar_double double_val;
    color_enum enum_val;
};

#endif /* TEST_TYPES_H */

/* test_types.h - Comprehensive GTY type definitions for gengtype coverage */

#ifndef TEST_TYPES_H
#define TEST_TYPES_H

/* Include GCC's GTY marker macros */
#ifndef GTY
#define GTY(x) 
#endif

/* Forward declaration for TYPE_UNDEFINED */
struct opaque;  /* This creates TYPE_UNDEFINED */

/* TYPE_SCALAR: Fundamental scalar types */
typedef enum GTY(()) color {
    RED,
    GREEN,
    BLUE
} color_t;

typedef int GTY(()) scalar_int;
typedef char GTY(()) scalar_char;
typedef long GTY(()) scalar_long;
typedef float GTY(()) scalar_float;
typedef double GTY(()) scalar_double;

/* TYPE_STRUCT: Basic struct with nested members */
struct GTY(()) basic_struct {
    int id;
    char name[32];
    float value;
};

/* TYPE_STRUCT with bit-fields */
struct GTY(()) bitfield_struct {
    unsigned int flag1 : 1;
    unsigned int flag2 : 2;
    unsigned int flag3 : 3;
    int value : 8;
};

/* TYPE_STRUCT with anonymous struct */
struct GTY(()) outer_struct {
    struct {
        int x;
        int y;
    } point;
    int id;
};

/* TYPE_STRUCT with complex nesting */
struct GTY(()) complex_struct {
    int id;
    struct basic_struct basic;
    struct bitfield_struct bits;
};

/* TYPE_USER_STRUCT: User-defined struct */
struct GTY((user)) user_struct {
    int user_id;
    char* user_name;
    void* user_data;
};

/* TYPE_UNION: Basic union */
union GTY(()) basic_union {
    int int_val;
    float float_val;
    char* string_val;
    double double_val;
};

/* TYPE_UNION with tag */
struct GTY(()) tagged_union_container {
    int tag;
    union {
        int as_int;
        float as_float;
        char* as_string;
    } GTY((desc ("%0.tag"))) value;
};

/* TYPE_POINTER: Various pointer types */
typedef struct basic_struct* GTY(()) struct_ptr;
typedef union basic_union* GTY(()) union_ptr;
typedef int* GTY(()) int_ptr;
typedef void* GTY(()) void_ptr;

/* TYPE_POINTER: Function pointer for TYPE_CALLBACK */
typedef void (*callback_func)(int, char*) GTY((callback));

/* TYPE_POINTER: Pointer to pointer */
typedef struct basic_struct** GTY(()) struct_ptr_ptr;

/* TYPE_ARRAY: Fixed-size arrays */
typedef int GTY(()) int_array[10];
typedef struct basic_struct GTY(()) struct_array[5];
typedef union basic_union GTY(()) union_array[3];

/* TYPE_ARRAY: Multi-dimensional array */
typedef int GTY(()) matrix[3][3];

/* TYPE_STRING: String types */
typedef char* GTY((length ("strlen(%0)"))) string_ptr;
typedef const char* GTY(()) const_string_ptr;

/* TYPE_CALLBACK: Struct with function pointer */
struct GTY(()) callback_container {
    int id;
    callback_func func GTY((skip));
    void (*another_func)(void) GTY((callback));
};

/* Chain structures for GTY options */
struct GTY((chain_next ("%h.next"), chain_prev ("%h.prev"))) chain_node {
    int value;
    struct chain_node* GTY((skip)) next;
    struct chain_node* GTY((skip)) prev;
};

/* Variable length array structure */
struct GTY(()) varray_struct {
    int length;
    int elements GTY((length ("%0.length"))) [1];
};

/* Union with nested struct */
union GTY(()) nested_union {
    struct {
        int x;
        int y;
    } point;
    struct {
        float r;
        float g;
        float b;
    } color;
};

#endif /* TEST_TYPES_H */

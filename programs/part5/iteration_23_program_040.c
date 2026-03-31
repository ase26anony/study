/* test_types.h - Comprehensive GTY type definitions for gengtype coverage */

#ifndef TEST_TYPES_H
#define TEST_TYPES_H

/* Include GCC's GTY marker macros */
#ifndef GTY
#define GTY(x) 
#endif

/* Forward declaration for TYPE_UNDEFINED case */
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

/* TYPE_STRUCT: Basic struct with various members */
struct GTY(()) basic_struct {
    int id;
    char name[32];
    float value;
    enum color color;
};

/* TYPE_STRUCT: Nested struct with anonymous struct */
struct GTY(()) complex_struct {
    struct basic_struct base;
    struct GTY(()) {
        int x;
        int y;
    } point;
    struct GTY(()) nested {
        int depth;
        struct complex_struct* GTY((skip)) parent;
    } inner;
};

/* TYPE_STRUCT: Struct with bit-fields */
struct GTY(()) bitfield_struct {
    unsigned int flag1 : 1;
    unsigned int flag2 : 2;
    unsigned int flag3 : 3;
    unsigned int padding : 26;
};

/* TYPE_USER_STRUCT: User-defined struct */
struct GTY((user)) user_struct {
    int user_data;
    void* GTY((skip)) user_ptr;
};

/* TYPE_UNION: Basic union */
union GTY(()) basic_union {
    int as_int;
    float as_float;
    char* GTY((length("strlen($)"))) as_string;
};

/* TYPE_UNION: Tagged union within a struct */
struct GTY(()) tagged_union_container {
    enum { INT_TYPE, FLOAT_TYPE, STRING_TYPE } tag;
    union GTY((desc("%0.tag"))) {
        int int_value;
        float float_value;
        char* GTY((length("strlen($)"))) string_value;
    } value;
};

/* TYPE_POINTER: Various pointer types */
typedef struct basic_struct* GTY(()) struct_ptr;
typedef union basic_union* GTY(()) union_ptr;
typedef int* GTY(()) int_ptr;
typedef void* GTY(()) void_ptr;
typedef struct basic_struct** GTY(()) ptr_to_ptr;

/* Function pointer type for TYPE_CALLBACK */
typedef void (*GTY(()) callback_func)(int, const char*);

/* TYPE_CALLBACK: Struct with function pointer */
struct GTY(()) callback_container {
    int id;
    callback_func GTY((skip)) handler;
    void (*GTY((skip)) another_handler)(void);
};

/* TYPE_ARRAY: Various array types */
typedef int GTY(()) int_array[10];
typedef struct basic_struct GTY(()) struct_array[5];
typedef union basic_union GTY(()) union_array[8];

/* Multi-dimensional array */
typedef int GTY(()) matrix[3][3];

/* TYPE_STRING: String types */
typedef char* GTY((length("strlen($)"))) string_ptr;
typedef const char* GTY((length("strlen($)"))) const_string_ptr;

/* Struct with string member */
struct GTY(()) string_struct {
    char* GTY((length("strlen($)"))) dynamic_string;
    const char* GTY((length("strlen($)"))) constant_string;
    char fixed_string[64];
};

/* TYPE_POINTER: Complex pointer chain */
struct GTY(()) pointer_chain {
    struct pointer_chain* GTY((chain_next("%h.next"))) next;
    struct pointer_chain* GTY((chain_prev("%h.prev"))) prev;
    struct basic_struct* GTY(()) data;
    int_array numbers;
};

/* Struct with array of pointers */
struct GTY(()) pointer_array_struct {
    struct basic_struct* GTY(()) ptrs[4];
    union basic_union* GTY(()) union_ptrs[2];
    callback_func GTY((skip)) handlers[3];
};

/* Union with struct member */
union GTY(()) union_with_struct {
    struct GTY(()) {
        int a;
        int b;
    } pair;
    long long value;
};

/* Forward declared struct that gets defined later */
struct GTY(()) forward_declared;

/* Complete the forward declaration */
struct GTY(()) forward_declared {
    int data;
    struct forward_declared* GTY((skip)) next;
};

#endif /* TEST_TYPES_H */

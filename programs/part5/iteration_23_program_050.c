/* test_types.h - Comprehensive GTY-annotated types for gengtype coverage */

#ifndef TEST_TYPES_H
#define TEST_TYPES_H

/* Include GCC's GTY macros */
#ifndef GTY
#define GTY(x) 
#endif

/* Forward declaration for TYPE_UNDEFINED case */
struct opaque;  /* TYPE_UNDEFINED */

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
    color_t color;
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
    unsigned int status : 2;
    
    /* Chain next pointer for GTY option */
    struct complex_struct *GTY((skip)) next;
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
    char *GTY((length("strlen($)"))) string_val;
    struct basic_struct *struct_ptr;
};

/* TYPE_UNION: Tagged union within struct */
struct GTY(()) tagged_union_container {
    enum { INT_TYPE, FLOAT_TYPE, STRING_TYPE } tag;
    union GTY(()) {
        int i;
        float f;
        char *GTY((length("strlen($)"))) s;
    } value;
};

/* TYPE_POINTER: Various pointer types */
typedef struct basic_struct *GTY(()) struct_ptr_t;
typedef union basic_union *GTY(()) union_ptr_t;
typedef void *GTY(()) void_ptr_t;
typedef int *GTY(()) int_ptr_t;
typedef int (*GTY(()) func_ptr_t)(int, char*);

/* TYPE_ARRAY: Fixed-size arrays */
typedef int GTY(()) int_array_10[10];
typedef struct basic_struct GTY(()) struct_array_5[5];
typedef char *GTY(()) string_array_3[3];

/* TYPE_ARRAY: Multi-dimensional array */
typedef int GTY(()) matrix_3x3[3][3];

/* TYPE_STRING: String types */
typedef char *GTY((length("strlen($)"))) string_t;
typedef const char *GTY((length("strlen($)"))) const_string_t;

/* TYPE_CALLBACK: Function pointer type */
typedef int (*GTY(()) callback_func)(int, void*);

/* Struct containing callback */
struct GTY(()) callback_container {
    callback_func func;
    void *GTY((skip)) user_data;
};

/* Complex type with all dependencies */
struct GTY(()) master_struct {
    /* Scalar types */
    int id;
    color_t color;
    
    /* Struct types */
    struct basic_struct basic;
    struct complex_struct *complex_ptr;
    
    /* Union type */
    union basic_union data;
    
    /* Array types */
    int_array_10 numbers;
    struct_array_5 items;
    matrix_3x3 transform;
    
    /* String types */
    string_t name;
    const_string_t const_name;
    
    /* Pointer types */
    void_ptr_t generic_ptr;
    func_ptr_t function_ptr;
    
    /* Callback */
    callback_func handler;
    
    /* Chain pointers for GTY graph */
    struct master_struct *GTY((chain_next("$->next"))) next;
    struct master_struct *GTY((chain_prev("$->prev"))) prev;
    
    /* Length field for arrays */
    size_t GTY((length("count"))) count;
    struct basic_struct *GTY((length("$->count"))) dynamic_array;
};

/* Global variable declarations */
extern struct master_struct GTY(()) global_master;
extern struct basic_struct GTY(()) global_basic_array[10];
extern union basic_union GTY(()) global_union;
extern string_t GTY((length("strlen($)"))) global_strings[5];

#endif /* TEST_TYPES_H */

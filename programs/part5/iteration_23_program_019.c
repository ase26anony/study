/* test_types.h - Comprehensive GTY-annotated types for gengtype coverage */

#ifndef TEST_TYPES_H
#define TEST_TYPES_H

/* Include GCC's GTY macros */
#ifndef GTY
#define GTY(x) 
#endif

/* Forward declaration for TYPE_UNDEFINED */
struct opaque_struct;

/* TYPE_SCALAR: Fundamental scalar types */
typedef enum GTY(()) color_enum {
    RED,
    GREEN,
    BLUE
} color_enum;

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
    color_enum color;
};

/* TYPE_STRUCT: Complex nested struct with bitfields and anonymous struct */
struct GTY(()) complex_struct {
    /* Bitfields */
    unsigned int flag1 : 1;
    unsigned int flag2 : 3;
    unsigned int : 4; /* Padding */
    
    /* Anonymous struct */
    struct GTY(()) {
        int x;
        int y;
    } point;
    
    /* Nested struct */
    struct basic_struct nested;
    
    /* Chain pointers for GTY options */
    struct complex_struct *GTY((chain_next)) next;
    struct complex_struct *GTY((chain_prev)) prev;
};

/* TYPE_USER_STRUCT: User-defined struct */
struct GTY((user)) user_struct {
    int user_data;
    void *GTY((skip)) user_pointer;
};

/* TYPE_UNION: Basic union */
union GTY(()) basic_union {
    int as_int;
    float as_float;
    char as_char;
    void *as_pointer;
};

/* TYPE_UNION: Tagged union within a struct */
struct GTY(()) tagged_union_container {
    enum { INT_TYPE, FLOAT_TYPE, STRING_TYPE } tag;
    
    union GTY(()) {
        int int_value;
        float float_value;
        char *GTY((tag("STRING_TYPE"))) string_value;
    } data;
};

/* TYPE_POINTER: Various pointer types */
typedef struct basic_struct *GTY(()) struct_pointer;
typedef union basic_union *GTY(()) union_pointer;
typedef int *GTY(()) int_pointer;
typedef void (*GTY(()) func_ptr)(int, char*);
typedef void *GTY(()) void_pointer;

/* TYPE_ARRAY: Various array types */
typedef int GTY(()) int_array[10];
typedef struct basic_struct GTY(()) struct_array[5];
typedef union basic_union GTY(()) union_array[8];

/* Multi-dimensional arrays */
typedef int GTY(()) matrix_2d[3][3];
typedef struct basic_struct GTY(()) struct_grid[2][2];

/* TYPE_STRING: String types */
typedef char *GTY((length("strlen($)"))) dynamic_string;
typedef const char *GTY(()) const_string;

/* TYPE_CALLBACK: Function pointer type */
typedef int (*GTY(()) compare_func)(const void*, const void*);

/* Struct using callback type */
struct GTY(()) callback_container {
    compare_func sorter;
    void *GTY((skip)) data;
    int data_length;
};

/* Struct with string pointer marked as string */
struct GTY(()) string_container {
    char *GTY((string)) name;
    dynamic_string description;
    const_string constant;
};

/* Linked list structure with all type kinds */
struct GTY(()) master_list {
    /* Scalar */
    int id;
    color_enum color;
    
    /* Struct */
    struct basic_struct item;
    
    /* Union */
    union basic_union value;
    
    /* Pointer */
    struct master_list *GTY((chain_next)) next;
    struct opaque_struct *opaque_ptr;  /* Pointer to undefined type */
    
    /* Array */
    int scores[5];
    
    /* String */
    char *GTY((string)) title;
    
    /* Callback */
    compare_func validator;
    
    /* User struct */
    struct user_struct *user_data;
    
    /* Anonymous union */
    union {
        int alt_id;
        char alt_name[20];
    } GTY(()) alternative;
};

/* Variable-length array structure */
struct GTY(()) var_array_struct {
    int length;
    int *GTY((length("$->length"))) variable_array;
    struct basic_struct *GTY((length("$->length"))) struct_array;
};

/* Extern declarations for gengtype to process */
extern struct basic_struct GTY(()) global_struct;
extern union basic_union GTY(()) global_union;
extern struct master_list *GTY(()) global_list;
extern int_array GTY(()) global_int_array;
extern dynamic_string GTY(()) global_strings[];
extern compare_func GTY(()) global_callbacks[3];

#endif /* TEST_TYPES_H */

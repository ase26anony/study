/* test_types.h - Comprehensive GTY type definitions for gengtype coverage */

#ifndef TEST_TYPES_H
#define TEST_TYPES_H

/* Include GCC's GTY marker macros */
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
typedef long GTY(()) scalar_long;
typedef float GTY(()) scalar_float;
typedef double GTY(()) scalar_double;
typedef char GTY(()) scalar_char;

/* TYPE_STRUCT: Basic struct with various members */
struct GTY(()) basic_struct {
    int id;
    char name[32];
    float value;
    enum color color;
};

/* TYPE_STRUCT: Nested struct with bit-fields */
struct GTY(()) nested_struct {
    struct basic_struct inner;
    unsigned int flags : 4;
    unsigned int count : 12;
    unsigned int : 16;  /* padding */
    
    /* Anonymous struct */
    struct GTY(()) {
        int x;
        int y;
    } point;
};

/* TYPE_STRUCT: Struct with pointer chain */
struct GTY((chain_next("next"))) linked_node {
    int data;
    struct linked_node *GTY((skip)) next;
    struct linked_node *GTY((skip)) prev;
};

/* TYPE_USER_STRUCT: User-defined struct */
struct GTY((user)) my_user_struct {
    int user_id;
    char *user_name;
};

/* TYPE_UNION: Basic union */
union GTY(()) basic_union {
    int as_int;
    float as_float;
    char as_char[4];
};

/* TYPE_UNION: Tagged union within struct */
struct GTY(()) tagged_container {
    enum { TAG_INT, TAG_FLOAT, TAG_STRING } tag;
    
    union GTY(()) {
        int int_value;
        float float_value;
        char *string_value;
    } data;
};

/* TYPE_POINTER: Various pointer types */
typedef struct basic_struct* GTY(()) struct_ptr;
typedef union basic_union* GTY(()) union_ptr;
typedef int* GTY(()) int_ptr;
typedef void* GTY(()) void_ptr;
typedef int (*GTY(()) func_ptr)(int, int);  /* Function pointer */

/* TYPE_ARRAY: Various array types */
typedef int GTY(()) int_array[10];
typedef struct basic_struct GTY(()) struct_array[5];
typedef int GTY(()) multi_dim_array[3][4][5];
typedef char* GTY(()) string_array[8];

/* TYPE_STRING: String types */
typedef char* GTY(()) string_ptr;
typedef const char* GTY(()) const_string_ptr;

/* TYPE_CALLBACK: Callback function pointer type */
typedef int (*GTY(()) callback_func)(void *data, int param);

/* Struct using callback */
struct GTY(()) callback_container {
    callback_func handler;
    void *user_data;
};

/* Complex struct with all type kinds */
struct GTY(()) master_struct {
    /* TYPE_SCALAR */
    int id;
    enum color color;
    
    /* TYPE_STRUCT */
    struct nested_struct nested;
    
    /* TYPE_UNION */
    union basic_union value;
    
    /* TYPE_POINTER */
    struct linked_node *list;
    void *generic_ptr;
    
    /* TYPE_ARRAY */
    int numbers[20];
    struct basic_struct items[5];
    
    /* TYPE_STRING */
    char *name;
    const char *const_name;
    
    /* TYPE_CALLBACK */
    callback_func on_event;
    
    /* Chain pointers */
    struct master_struct *GTY((skip)) next;
    struct master_struct *GTY((skip)) prev;
};

/* Variable-length array struct */
struct GTY(()) varray_struct {
    int length;
    int data[1];  /* Variable length */
};

/* Struct with desc tag */
struct GTY((desc("%1.type"))) desc_struct {
    enum { TYPE_A, TYPE_B } type;
    union {
        int a;
        float b;
    } value;
};

#endif /* TEST_TYPES_H */

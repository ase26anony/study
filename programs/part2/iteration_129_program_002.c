#ifndef TEST_TYPES_1_H
#define TEST_TYPES_1_H

/* Undefined Type - TYPE_UNDEFINED */
typedef void *undefined_type_t;
struct forward_declared_struct; /* Another undefined type */

/* Scalar Types - TYPE_SCALAR */
typedef int scalar_int_t;
typedef char scalar_char_t;
typedef float scalar_float_t;
typedef double scalar_double_t;

enum color { RED, GREEN, BLUE };
typedef enum color color_t;

/* String Type - TYPE_STRING */
typedef const char *string_type;
typedef char *mutable_string_type;

/* Struct Types - TYPE_STRUCT */
struct plain_struct {
    int field1;
    char field2;
    float field3;
};

typedef struct {
    int x;
    int y;
} point_t;

/* User Struct Types - TYPE_USER_STRUCT */
struct GTY((user)) user_struct {
    int user_data;
    char *user_name;
};

typedef struct GTY((user)) user_typedef_struct {
    long id;
    void *user_ptr;
} user_typedef_t;

/* Union Types - TYPE_UNION */
union data_union {
    int int_val;
    float float_val;
    char char_val;
    double double_val;
};

typedef union {
    unsigned int bits;
    struct {
        unsigned char r;
        unsigned char g;
        unsigned char b;
        unsigned char a;
    } rgba;
} color_union_t;

/* Pointer Types - TYPE_POINTER */
typedef int *int_ptr_t;
typedef struct plain_struct *struct_ptr_t;
typedef void (*void_func_ptr_t)(void);

/* Array Types - TYPE_ARRAY */
typedef int int_array_10_t[10];
typedef char name_buffer_t[256];
extern float global_float_array[50];

/* Callback Types - TYPE_CALLBACK */
typedef int (*comparator_t)(const void *, const void *);
typedef void (*callback_func_t)(int, char *);
typedef int (*binary_op_t)(int, int);

#endif /* TEST_TYPES_1_H */

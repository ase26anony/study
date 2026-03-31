#ifndef TEST_TYPES_1_H
#define TEST_TYPES_1_H

/* Undefined Type - TYPE_UNDEFINED */
typedef void *undefined_type_t;
struct forward_declared;  /* Another undefined type */

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

/* User Struct Type - TYPE_USER_STRUCT */
struct GTY((user)) user_defined_struct {
    int user_data;
    void *user_pointer;
};

/* Union Type - TYPE_UNION */
union data_union {
    int int_val;
    float float_val;
    char char_val;
};

typedef union {
    long long_val;
    double double_val;
} number_union_t;

/* Pointer Types - TYPE_POINTER */
typedef int *int_ptr_t;
typedef struct plain_struct *struct_ptr_t;
typedef void (*void_func_ptr_t)(void);

/* Array Types - TYPE_ARRAY */
typedef int int_array_10[10];
typedef char char_array_20[20];
typedef struct plain_struct struct_array_5[5];

/* Callback Types - TYPE_CALLBACK */
typedef int (*comparator_t)(const void *, const void *);
typedef void (*callback_func_t)(int, char *);
typedef int (*transform_t)(int);

/* Language-Specific Struct Type - TYPE_LANG_STRUCT */
struct GTY((lang_type)) language_struct {
    int lang_specific_field;
    void *lang_data;
};

/* Mixed complex type */
struct complex_type {
    int id;
    string_type name;
    int_array_10 scores;
    comparator_t compare_func;
    union data_union data;
};

#endif /* TEST_TYPES_1_H */

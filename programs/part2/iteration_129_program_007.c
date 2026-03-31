#ifndef TEST_TYPES_1_H
#define TEST_TYPES_1_H

/* Undefined Type - TYPE_UNDEFINED */
typedef void *undefined_type_t;
struct forward_declared_struct;  /* Another undefined type */

/* Scalar Types - TYPE_SCALAR */
typedef int scalar_int_t;
typedef char scalar_char_t;
typedef float scalar_float_t;
typedef double scalar_double_t;

/* Enum as scalar */
enum color { RED, GREEN, BLUE };
typedef enum color color_t;

/* String Type - TYPE_STRING */
typedef const char *string_type;
typedef char *mutable_string_t;

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
struct GTY((user)) user_defined_struct {
    int user_data;
    void *user_pointer;
};

typedef struct GTY((user)) {
    int id;
    string_type name;
} user_type_t;

/* Union Types - TYPE_UNION */
union data_union {
    int int_val;
    float float_val;
    char char_val;
    double double_val;
};

typedef union {
    long long_val;
    void *ptr_val;
} variant_t;

/* Pointer Types - TYPE_POINTER */
typedef int *int_ptr_t;
typedef struct plain_struct *struct_ptr_t;
typedef void (*void_func_ptr)(void);

/* Array Types - TYPE_ARRAY */
typedef int int_array_10[10];
typedef char char_matrix[5][5];
extern float global_array[100];

/* Callback Types - TYPE_CALLBACK */
typedef int (*callback_func)(int, char*);
typedef void (*simple_callback)(void);
typedef int (*complex_callback)(callback_func, void*);

/* Language-Specific Struct Types - TYPE_LANG_STRUCT */
struct GTY((lang_type)) lang_specific_struct {
    int lang_field;
    void *lang_data;
};

typedef struct GTY((lang_type)) {
    int token;
    int value;
} lang_token_t;

#endif /* TEST_TYPES_1_H */

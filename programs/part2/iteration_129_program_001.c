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

struct nested_struct {
    struct plain_struct inner;
    point_t point;
};

/* User Struct Types - TYPE_USER_STRUCT */
struct GTY((user)) user_struct {
    int user_data;
    void *user_pointer;
};

typedef struct GTY((user)) user_typedef_struct {
    double value;
    string_type name;
} user_typedef_t;

/* Union Types - TYPE_UNION */
union data_union {
    int i;
    float f;
    char c;
    void *p;
};

typedef union {
    long long_val;
    double double_val;
} number_union_t;

/* Pointer Types - TYPE_POINTER */
typedef int *int_ptr_t;
typedef struct plain_struct *struct_ptr_t;
typedef void (*void_func_ptr_t)(void);
typedef const char *const *string_ptr_ptr_t;

/* Array Types - TYPE_ARRAY */
typedef int int_array_10[10];
typedef char name_buffer[256];
typedef struct plain_struct struct_array[5];
typedef int multi_dim_array[3][4][5];

/* Callback Types - TYPE_CALLBACK */
typedef int (*callback_t)(int, void*);
typedef void (*simple_callback_t)(void);
typedef char *(*string_callback_t)(const char*);
typedef int (*comparator_t)(const void*, const void*);

/* Language-Specific Struct Types - TYPE_LANG_STRUCT */
struct GTY((lang_type)) lang_struct {
    int lang_field;
    void *lang_data;
};

typedef struct GTY((lang_type)) {
    int type_code;
    const char *type_name;
} lang_type_info_t;

/* Mixed complex type */
struct GTY((user)) complex_user_type {
    int id;
    string_type name;
    callback_t handler;
    struct plain_struct *data;
    int_array_10 scores;
    union data_union value;
};

#endif /* TEST_TYPES_1_H */

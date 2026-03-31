#ifndef TEST_TYPES_1_H
#define TEST_TYPES_1_H

/* Undefined Type - forward declaration without definition */
typedef void *undefined_type_t;
struct undefined_struct;  /* Another undefined type */

/* Scalar Types */
typedef int scalar_int_t;
typedef char scalar_char_t;
typedef float scalar_float_t;
typedef double scalar_double_t;

/* Enum scalar type */
enum color { RED, GREEN, BLUE };
typedef enum color color_t;

/* String Type */
typedef const char *string_type;
typedef char *mutable_string_type;

/* Struct Types */
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

/* User Struct Type with GTY marker */
struct GTY((user)) user_struct {
    int user_data;
    char *user_name;
};

typedef struct GTY((user)) user_struct_typedef {
    double value;
    int id;
} user_struct_t;

/* Union Types */
union basic_union {
    int int_val;
    float float_val;
    char char_val;
};

typedef union {
    long long_val;
    double double_val;
} union_t;

/* Pointer Types */
int *int_ptr;
struct plain_struct *struct_ptr;
void **void_ptr_ptr;
undefined_type_t undefined_ptr;

/* Array Types */
int int_array[10];
char char_array[256];
struct plain_struct struct_array[5];
float multi_dim_array[3][4][5];

/* Callback Types */
typedef int (*simple_callback)(int);
typedef void (*complex_callback)(int, char*, float);
typedef int (*callback_returning_ptr)(void);

/* Language-specific Struct Type */
struct GTY((lang_type)) lang_struct {
    int lang_field;
    void *lang_data;
};

typedef struct GTY((lang_type)) lang_struct_typedef {
    char *name;
    int type;
} lang_struct_t;

#endif /* TEST_TYPES_1_H */

#ifndef TEST_TYPES_1_H
#define TEST_TYPES_1_H

/* Undefined Type (TYPE_UNDEFINED) */
typedef void *undefined_type_t;
struct forward_declared;  /* Another undefined type */

/* Scalar Types (TYPE_SCALAR) */
typedef int scalar_int_t;
typedef char scalar_char_t;
typedef float scalar_float_t;
enum color { RED, GREEN, BLUE };
typedef enum color color_t;

/* String Type (TYPE_STRING) */
typedef const char *string_type;

/* Struct Types (TYPE_STRUCT) */
struct plain_struct {
    int field1;
    char field2;
};

typedef struct {
    int x;
    int y;
} point_t;

/* User Struct Types (TYPE_USER_STRUCT) */
struct GTY((user)) user_struct {
    int user_data;
    char *user_name;
};

typedef struct GTY((user)) user_typedef_struct {
    double value;
    int id;
} user_typedef_t;

/* Union Types (TYPE_UNION) */
union data_union {
    int int_val;
    float float_val;
    char char_val;
};

typedef union {
    long long_val;
    double double_val;
} number_union_t;

/* Pointer Types (TYPE_POINTER) */
int *int_ptr;
struct plain_struct *struct_ptr;
void **void_ptr_ptr;

/* Array Types (TYPE_ARRAY) */
int int_array[10];
char char_array[20];
struct plain_struct struct_array[5];

/* Callback Types (TYPE_CALLBACK) */
typedef int (*simple_callback)(int);
typedef void (*complex_callback)(int, char*, float);

/* Language-Specific Struct Types (TYPE_LANG_STRUCT) */
struct GTY((lang_type)) lang_struct {
    int lang_field;
    void *lang_data;
};

typedef struct GTY((lang_type)) {
    int type_id;
    const char *type_name;
} lang_typedef_t;

#endif /* TEST_TYPES_1_H */

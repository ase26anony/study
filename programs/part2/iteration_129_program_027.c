#ifndef TEST_TYPES_1_H
#define TEST_TYPES_1_H

/* Undefined Type (TYPE_UNDEFINED) */
typedef void *undefined_type_t;
struct forward_declared;  /* Another undefined type */

/* Scalar Types (TYPE_SCALAR) */
typedef int scalar_int_t;
typedef char scalar_char_t;
typedef float scalar_float_t;
typedef double scalar_double_t;

enum color { RED, GREEN, BLUE };
typedef enum color color_t;

/* String Type (TYPE_STRING) */
typedef const char *string_type;
typedef char *mutable_string_type;

/* Struct Types (TYPE_STRUCT) */
struct plain_struct {
    int field1;
    char field2;
};

typedef struct {
    float x;
    float y;
} point_t;

struct nested_struct {
    struct plain_struct inner;
    point_t point;
};

/* User Struct Types (TYPE_USER_STRUCT) */
struct GTY((user)) user_struct {
    int data;
    void *user_data;
};

typedef struct GTY((user)) user_typedef_struct {
    long id;
    char name[32];
} user_typedef_t;

/* Union Types (TYPE_UNION) */
union data_union {
    int i;
    float f;
    char c;
};

typedef union {
    long l;
    double d;
} number_union_t;

/* Pointer Types (TYPE_POINTER) */
int *int_ptr;
struct plain_struct *struct_ptr;
void **void_ptr_ptr;
user_typedef_t *user_struct_ptr;

/* Array Types (TYPE_ARRAY) */
int int_array[10];
char char_array[256];
struct plain_struct struct_array[5];
float multi_dim_array[3][4][5];

/* Callback Types (TYPE_CALLBACK) */
typedef int (*simple_callback_t)(int);
typedef void (*complex_callback_t)(int, char *, struct plain_struct *);
int (*direct_func_ptr)(float, double);

/* Language-Specific Struct Types (TYPE_LANG_STRUCT) */
struct GTY((lang_type)) lang_struct {
    int lang_field;
    void *lang_data;
};

struct GTY((lang_type)) another_lang_struct {
    char *name;
    int token;
    struct lang_struct *next;
};

#endif /* TEST_TYPES_1_H */

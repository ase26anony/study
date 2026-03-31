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
struct my_struct {
    int a;
    float b;
};

typedef struct {
    int x;
    int y;
} point_t;

/* User Struct Types (TYPE_USER_STRUCT) */
struct GTY((user)) user_struct {
    int data;
    void *user_data;
};

/* Union Types (TYPE_UNION) */
union my_union {
    int i;
    float f;
    char c;
};

typedef union {
    long l;
    double d;
} number_union_t;

/* Pointer Types (TYPE_POINTER) */
typedef int *int_ptr_t;
typedef struct my_struct *struct_ptr_t;
typedef void (*void_func_ptr_t)(void);

/* Array Types (TYPE_ARRAY) */
typedef int int_array_t[10];
typedef char name_t[50];

/* Callback Types (TYPE_CALLBACK) */
typedef int (*callback_t)(int, void*);
typedef void (*simple_callback_t)(void);

/* Language-Specific Struct Types (TYPE_LANG_STRUCT) */
struct GTY((lang_type)) lang_struct {
    int lang_field;
    void *lang_data;
};

#endif /* TEST_TYPES_1_H */

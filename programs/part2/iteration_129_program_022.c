#ifndef TEST_TYPES_1_H
#define TEST_TYPES_1_H

/* Undefined Type - forward declaration without definition */
typedef void *undefined_type_t;
struct undefined_struct;  /* Another undefined type */

/* Scalar Types */
typedef int scalar_int_t;
typedef char scalar_char_t;
typedef float scalar_float_t;
enum scalar_enum { ENUM_A, ENUM_B, ENUM_C };

/* String Type */
typedef const char *string_type;

/* Struct Types */
struct plain_struct {
    int field1;
    char field2;
};

typedef struct {
    float x;
    float y;
} typedef_struct_t;

/* User Struct Type (with GTY marker) */
struct GTY((user)) user_struct_type {
    int user_data;
    char *user_name;
};

/* Union Type */
union my_union {
    int i;
    float f;
    char c;
};

/* Pointer Types */
typedef int *int_ptr_t;
typedef struct plain_struct *struct_ptr_t;
typedef void (*void_func_ptr)(void);

/* Array Types */
typedef int int_array_10[10];
typedef char char_array_5[5];
extern float extern_array[20];

/* Callback Types */
typedef int (*callback_func_t)(int, char*);
typedef void (*simple_callback_t)(void);

/* Language-Specific Struct Type */
struct GTY((lang_type)) lang_specific_struct {
    int lang_field;
    void *lang_data;
};

/* Mixed complex type using multiple categories */
struct GTY((user)) complex_user_type {
    callback_func_t handler;      /* TYPE_CALLBACK */
    string_type name;             /* TYPE_STRING */
    int_array_10 buffer;          /* TYPE_ARRAY */
    struct plain_struct *next;    /* TYPE_POINTER -> TYPE_STRUCT */
};

#endif /* TEST_TYPES_1_H */

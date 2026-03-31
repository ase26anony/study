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
    double x;
    double y;
} typedef_struct_t;

/* User Struct Type (with GTY marker) */
struct GTY((user)) user_struct_type {
    int user_data;
    void *user_ptr;
};

/* Union Type */
union basic_union {
    int int_val;
    float float_val;
    char char_val;
};

/* Pointer Types */
typedef int *int_ptr_t;
typedef struct plain_struct *struct_ptr_t;
typedef void (*void_func_ptr_t)(void);

/* Array Types */
typedef int fixed_array_t[10];
extern char global_array[256];

/* Callback Type */
typedef int (*callback_type)(int, char*);

/* Language-Specific Struct Type */
struct GTY((lang_type)) lang_specific_struct {
    int lang_field;
    void *lang_data;
};

#endif /* TEST_TYPES_1_H */

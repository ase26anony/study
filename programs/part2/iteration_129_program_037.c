#ifndef TEST_TYPES_1_H
#define TEST_TYPES_1_H

/* Undefined Type - forward declaration without definition */
typedef void *undefined_type_t;
struct undefined_struct;  /* Another undefined type */

/* Scalar Types */
typedef int scalar_int_t;
typedef char scalar_char_t;
typedef float scalar_float_t;
enum scalar_enum { ENUM_VAL1, ENUM_VAL2 };

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
} point_t;

/* User Struct Type with GTY marker */
struct GTY((user)) user_defined_struct {
    int user_data;
    char *user_name;
};

/* Union Type */
union data_union {
    int int_val;
    float float_val;
    char char_val;
};

/* Pointer Types */
int *int_ptr;
struct plain_struct *struct_ptr;
void **void_ptr_ptr;

/* Array Types */
int int_array[10];
char char_array[256];
struct plain_struct struct_array[5];

/* Callback Type */
typedef int (*callback_func)(int, char*);
typedef void (*simple_callback)(void);

/* Language-Specific Struct Type */
struct GTY((lang_type)) lang_specific_struct {
    int lang_field;
    void *lang_data;
};

#endif /* TEST_TYPES_1_H */

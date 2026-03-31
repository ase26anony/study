#ifndef TEST_TYPES_1_H
#define TEST_TYPES_1_H

/* Undefined Type - forward declaration without definition */
typedef void *undefined_type_t;
struct undefined_struct;

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
struct GTY((user)) user_struct {
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
int *int_ptr;
struct plain_struct *struct_ptr;
void **void_ptr_ptr;

/* Array Types */
int int_array[10];
char char_array[20];
struct plain_struct struct_array[5];

/* Callback Type */
typedef int (*callback_t)(int, char*);
typedef void (*void_callback_t)(void);

/* Language-Specific Struct Type */
struct GTY((lang_type)) lang_struct {
    int lang_field;
    void *lang_data;
};

#endif /* TEST_TYPES_1_H */

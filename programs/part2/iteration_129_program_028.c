#ifndef TEST_TYPES_1_H
#define TEST_TYPES_1_H

/* Undefined Type - forward declaration without definition */
typedef void *undefined_type_t;
struct undefined_struct;  /* Another undefined type */

/* Scalar Types */
typedef int scalar_int_t;
typedef char scalar_char_t;
typedef float scalar_float_t;
enum color { RED, GREEN, BLUE };
typedef enum color color_t;

/* String Type */
typedef const char *string_type;

/* Struct Types */
struct my_struct {
    int a;
    float b;
};

typedef struct {
    int x;
    int y;
} point_t;

/* User Struct Type (with GTY marker) */
struct GTY((user)) user_struct {
    int data;
    void *ptr;
};

/* Union Type */
union my_union {
    int i;
    float f;
    char c;
};

/* Pointer Types */
typedef int *int_ptr_t;
typedef struct my_struct *struct_ptr_t;
typedef void (*void_func_ptr)(void);

/* Array Types */
typedef int int_array_10[10];
typedef char char_matrix[5][5];

/* Callback Type */
typedef int (*callback_t)(int, void*);
typedef void (*simple_callback)(void);

/* Language-Specific Struct Type */
struct GTY((lang_type)) lang_struct {
    int lang_field;
    void *lang_data;
};

#endif /* TEST_TYPES_1_H */

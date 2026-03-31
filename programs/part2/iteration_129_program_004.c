#ifndef TYPES_HEADER_1_H
#define TYPES_HEADER_1_H

/* Undefined Type - forward declaration without definition */
typedef void *undefined_type_t;
struct undefined_struct; /* Another undefined type */

/* Scalar Types */
typedef int scalar_int_t;
typedef char scalar_char_t;
typedef float scalar_float_t;
enum color { RED, GREEN, BLUE };
typedef enum color color_t;

/* String Type */
typedef const char *string_type;
typedef char *mutable_string_type;

/* Struct Types */
struct my_struct {
    int a;
    float b;
    char c;
};

typedef struct {
    int x;
    int y;
} point_t;

/* User Struct Type with GTY marker */
struct GTY((user)) user_struct {
    int data;
    void *user_data;
};

/* Union Types */
union my_union {
    int i;
    float f;
    char c;
};

typedef union {
    long l;
    double d;
} number_union_t;

/* Pointer Types */
typedef int *int_ptr_t;
typedef struct my_struct *struct_ptr_t;
typedef void (*void_func_ptr_t)(void);

/* Array Types */
typedef int int_array_10[10];
typedef char name_array[50];

/* Callback Types */
typedef int (*callback_t)(int, void*);
typedef void (*simple_callback_t)(void);

/* Language-Specific Struct Type */
struct GTY((lang_type)) lang_struct {
    int lang_field;
    void *lang_data;
};

#endif /* TYPES_HEADER_1_H */

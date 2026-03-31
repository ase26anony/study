#ifndef TYPES_PRIMARY_H
#define TYPES_PRIMARY_H

/* Undefined Type */
typedef void *undefined_type_t;
struct forward_declared;  /* Another undefined type */

/* Scalar Types */
typedef int scalar_int_t;
typedef char scalar_char_t;
typedef float scalar_float_t;
typedef double scalar_double_t;

enum color { RED, GREEN, BLUE };
typedef enum color color_t;

/* String Type */
typedef const char *string_type;
typedef char *mutable_string_type;

/* Struct Types */
struct plain_struct {
    int field1;
    char field2;
};

typedef struct {
    int x;
    int y;
} point_t;

struct GTY((user)) user_defined_struct {
    int user_data;
    void *user_pointer;
};

/* Union Types */
union data_union {
    int int_val;
    float float_val;
    char char_val;
};

typedef union {
    long long_val;
    double double_val;
} number_union_t;

/* Pointer Types */
typedef int *int_ptr_t;
typedef struct plain_struct *struct_ptr_t;
typedef void (*void_func_ptr)(void);

/* Array Types */
typedef int int_array_10[10];
typedef char string_buffer[256];

/* Callback Types */
typedef int (*comparator_t)(const void *, const void *);
typedef void (*callback_func)(int, char *);

/* Language-Specific Struct */
struct GTY((lang_type)) language_struct {
    int lang_specific_field;
    void *lang_data;
};

#endif /* TYPES_PRIMARY_H */

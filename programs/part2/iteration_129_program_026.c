#ifndef FILE1_TYPES_H
#define FILE1_TYPES_H

/* Undefined Type */
typedef void *undefined_type_t;
struct forward_declared;  /* Another undefined type */

/* Scalar Types */
typedef int scalar_int_t;
typedef char scalar_char_t;
typedef float scalar_float_t;
enum color { RED, GREEN, BLUE };
typedef enum color color_t;

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

/* User Struct Type (with GTY marker) */
struct GTY((user)) user_struct {
    int user_data;
    char *user_name;
};

/* Union Type */
union data_union {
    int int_val;
    float float_val;
    char *str_val;
};

/* Pointer Types */
typedef int *int_ptr_t;
typedef struct plain_struct *struct_ptr_t;
typedef void (*void_func_ptr)(void);

/* Array Types */
typedef int int_array_10[10];
typedef char name_array[50];

/* Callback Type */
typedef int (*callback_t)(int, void*);
typedef void (*simple_callback)(void);

/* Language-Specific Struct Type */
struct GTY((lang_type)) lang_struct {
    int lang_field;
    void *lang_data;
};

#endif /* FILE1_TYPES_H */

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
typedef struct my_struct my_struct_t;

struct another_struct {
    char c;
    double d;
};

/* User Struct Type (TYPE_USER_STRUCT) */
struct GTY((user)) user_struct {
    int data;
    void *user_data;
};

/* Union Type (TYPE_UNION) */
union my_union {
    int i;
    float f;
    char c;
};

/* Pointer Types (TYPE_POINTER) */
typedef int *int_ptr_t;
typedef struct my_struct *struct_ptr_t;
typedef void (*void_func_ptr)(void);

/* Array Type (TYPE_ARRAY) */
typedef int int_array_t[10];
typedef char char_matrix_t[5][20];

/* Callback Type (TYPE_CALLBACK) */
typedef int (*callback_t)(int, void*);
typedef void (*simple_callback)(void);

/* Language-Specific Struct Type (TYPE_LANG_STRUCT) */
struct GTY((lang_type)) lang_struct {
    int lang_field;
    void *lang_data;
};

#endif /* TEST_TYPES_1_H */

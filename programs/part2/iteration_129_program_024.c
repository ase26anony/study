#ifndef TEST_TYPES_1_H
#define TEST_TYPES_1_H

/* Undefined Type - TYPE_UNDEFINED */
typedef void *undefined_type_t;
struct forward_declared;  /* Another undefined type */

/* Scalar Types - TYPE_SCALAR */
typedef int scalar_int_t;
typedef char scalar_char_t;
typedef float scalar_float_t;
typedef double scalar_double_t;

/* Enum as scalar */
enum color { RED, GREEN, BLUE };
typedef enum color color_t;

/* String Type - TYPE_STRING */
typedef const char *string_type;
typedef char *mutable_string_t;

/* Struct Types - TYPE_STRUCT */
struct plain_struct {
    int field1;
    char field2;
    float field3;
};

typedef struct {
    int x;
    int y;
} point_t;

struct nested_struct {
    struct plain_struct inner;
    point_t point;
};

/* User Struct Types - TYPE_USER_STRUCT */
struct GTY((user)) user_struct {
    int user_data;
    void *user_ptr;
};

typedef struct GTY((user)) user_typedef_t {
    long id;
    char name[32];
} user_typedef_t;

/* Union Types - TYPE_UNION */
union data_union {
    int i;
    float f;
    char c;
    void *p;
};

typedef union {
    long long_val;
    double double_val;
} number_union_t;

/* Pointer Types - TYPE_POINTER */
typedef int *int_ptr_t;
typedef struct plain_struct *struct_ptr_t;
typedef void (*void_func_ptr)(void);

/* Array Types - TYPE_ARRAY */
typedef int int_array_10[10];
typedef char string_buffer[256];
extern float global_array[100];

/* Callback Types - TYPE_CALLBACK */
typedef int (*comparator_t)(const void *, const void *);
typedef void (*callback_func)(int, char *);
typedef int (*math_operation)(int, int);

/* Language-Specific Struct Types - TYPE_LANG_STRUCT */
struct GTY((lang_type)) lang_struct {
    int lang_field;
    void *lang_data;
};

struct GTY((lang_type)) another_lang_struct {
    long token;
    struct GTY((lang_type)) lang_struct *next;
};

#endif /* TEST_TYPES_1_H */

#ifndef TEST_TYPES_1_H
#define TEST_TYPES_1_H

/* Undefined type - TYPE_UNDEFINED */
typedef void *undefined_type_t;
struct forward_declared;  /* Another undefined type */

/* Scalar types - TYPE_SCALAR */
typedef int scalar_int_t;
typedef char scalar_char_t;
typedef float scalar_float_t;
enum color { RED, GREEN, BLUE };  /* Enum is also scalar */

/* String type - TYPE_STRING */
typedef const char *string_type;

/* Struct types - TYPE_STRUCT */
struct plain_struct {
    int field1;
    char field2;
};

typedef struct {
    float x;
    float y;
} point_t;

/* User struct type - TYPE_USER_STRUCT */
struct GTY((user)) user_defined_struct {
    int user_data;
    char *user_name;
};

/* Union type - TYPE_UNION */
union data_union {
    int int_val;
    float float_val;
    char *string_val;
};

/* Pointer types - TYPE_POINTER */
typedef int *int_ptr_t;
typedef struct plain_struct *struct_ptr_t;
typedef void (*void_func_ptr)(void);

/* Array type - TYPE_ARRAY */
typedef int int_array_t[10];
extern char global_buffer[256];

/* Callback type - TYPE_CALLBACK */
typedef int (*comparison_callback)(const void *, const void *);

/* Language-specific struct - TYPE_LANG_STRUCT */
struct GTY((lang_type)) language_struct {
    int lang_specific_field;
    void *lang_data;
};

#endif /* TEST_TYPES_1_H */

#ifndef TYPES_HEADER_1_H
#define TYPES_HEADER_1_H

/* Undefined Type (TYPE_UNDEFINED) */
typedef void *undefined_type_t;
struct forward_declared_struct; /* Another undefined type */

/* Scalar Types (TYPE_SCALAR) */
typedef int scalar_int_t;
typedef char scalar_char_t;
typedef float scalar_float_t;
typedef double scalar_double_t;

/* Enum scalar type */
enum color { RED, GREEN, BLUE };
typedef enum color color_t;

/* String Type (TYPE_STRING) */
typedef const char *string_type;
typedef char *mutable_string_type;

/* Struct Types (TYPE_STRUCT) */
struct plain_struct {
    int field1;
    char field2;
    float field3;
};

typedef struct {
    int x;
    int y;
} point_t;

/* User Struct Type (TYPE_USER_STRUCT) */
struct GTY((user)) user_struct {
    int user_data;
    char *user_name;
};

typedef struct GTY((user)) user_typedef_struct {
    long id;
    void *user_ptr;
} user_typedef_t;

/* Union Type (TYPE_UNION) */
union data_union {
    int int_val;
    float float_val;
    char char_val;
    double double_val;
};

typedef union {
    unsigned int flags;
    struct {
        unsigned int flag1:1;
        unsigned int flag2:1;
        unsigned int flag3:1;
    } bits;
} flags_union_t;

/* Pointer Types (TYPE_POINTER) */
typedef int *int_ptr_t;
typedef struct plain_struct *struct_ptr_t;
typedef void (*void_func_ptr_t)(void);

/* Array Types (TYPE_ARRAY) */
typedef int int_array_10_t[10];
typedef char name_array_t[50];
typedef struct plain_struct struct_array_t[5];

/* Callback Types (TYPE_CALLBACK) */
typedef int (*callback_t)(int, char*);
typedef void (*simple_callback_t)(void);
typedef int (*complex_callback_t)(int, float, void*);

/* Language-Specific Struct Type (TYPE_LANG_STRUCT) */
struct GTY((lang_type)) lang_struct {
    int lang_field;
    void *lang_data;
};

typedef struct GTY((lang_type)) {
    int lang_id;
    const char *lang_name;
} lang_typedef_t;

#endif /* TYPES_HEADER_1_H */

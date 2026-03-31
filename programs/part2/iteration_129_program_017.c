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
typedef char *mutable_string_t;

/* Struct Types (TYPE_STRUCT) */
struct plain_struct {
    int field1;
    char field2;
};

typedef struct {
    double x;
    double y;
} point_t;

/* User Struct Types (TYPE_USER_STRUCT) */
struct GTY((user)) user_struct {
    int user_data;
    void *user_ptr;
};

typedef struct GTY((user)) {
    long id;
    string_type name;
} user_typedef_t;

/* Union Types (TYPE_UNION) */
union data_union {
    int int_val;
    float float_val;
    char char_val;
};

typedef union {
    long l;
    double d;
} number_union_t;

/* Pointer Types (TYPE_POINTER) */
typedef int *int_ptr_t;
typedef struct plain_struct *struct_ptr_t;
typedef void (*void_func_ptr)(void);

/* Array Types (TYPE_ARRAY) */
typedef int int_array_10[10];
typedef char name_buffer[256];
extern float global_array[100];

/* Callback Types (TYPE_CALLBACK) */
typedef int (*callback_t)(int, void*);
typedef void (*simple_callback)(void);
typedef char *(*string_callback)(const char*);

/* Language-Specific Struct Types (TYPE_LANG_STRUCT) */
struct GTY((lang_type)) lang_struct {
    int lang_field;
    void *lang_data;
};

typedef struct GTY((lang_type)) {
    int type_id;
    const char *type_name;
} lang_type_t;

#endif /* TEST_TYPES_1_H */

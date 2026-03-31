#ifndef TEST_TYPES_1_H
#define TEST_TYPES_1_H

/* Undefined Type (TYPE_UNDEFINED) */
typedef void *undefined_type_t;
struct forward_declared_struct;  /* Another undefined type */

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

/* User Struct Type (TYPE_USER_STRUCT) */
struct GTY((user)) user_struct {
    int user_data;
    void *user_ptr;
};

/* Union Type (TYPE_UNION) */
union data_union {
    int int_val;
    float float_val;
    char char_val;
};

/* Pointer Types (TYPE_POINTER) */
typedef int *int_ptr_t;
typedef struct plain_struct *struct_ptr_t;
typedef void (*void_func_ptr)(void);

/* Array Types (TYPE_ARRAY) */
typedef int int_array_10[10];
extern char global_buffer[256];

/* Callback Types (TYPE_CALLBACK) */
typedef int (*comparator_t)(const void *, const void *);
typedef void (*event_handler_t)(int event_id, void *data);

/* Language-Specific Struct (TYPE_LANG_STRUCT) */
struct GTY((lang_type)) lang_specific_struct {
    int lang_field;
    void *lang_data;
};

#endif /* TEST_TYPES_1_H */

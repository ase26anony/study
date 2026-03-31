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

/* User Struct Types (TYPE_USER_STRUCT) */
struct GTY((user)) user_struct {
    int data;
    void *user_data;
};

struct GTY((user)) another_user_struct {
    long id;
    char name[32];
};

/* Union Types (TYPE_UNION) */
union my_union {
    int i;
    float f;
    char c;
};

typedef union my_union my_union_t;

/* Pointer Types (TYPE_POINTER) */
int *int_ptr;
struct my_struct *struct_ptr;
void **void_ptr_ptr;
my_union_t *union_ptr;

/* Array Types (TYPE_ARRAY) */
int int_array[10];
char char_array[256];
float float_array[5][5];  /* Multi-dimensional array */
struct my_struct struct_array[20];

/* Callback Types (TYPE_CALLBACK) */
typedef int (*callback_t)(int, void*);
typedef void (*simple_callback)(void);
typedef char* (*string_callback)(const char*);

/* Language-Specific Struct Types (TYPE_LANG_STRUCT) */
struct GTY((lang_type)) lang_struct {
    int lang_field;
    void *lang_data;
};

struct GTY((lang_type)) another_lang_struct {
    unsigned long flags;
    const char *name;
};

#endif /* TEST_TYPES_1_H */

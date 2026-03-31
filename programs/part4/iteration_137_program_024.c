/* test_structures.h - Header with diverse type definitions for gengtype coverage */

#ifndef TEST_STRUCTURES_H
#define TEST_STRUCTURES_H

/* TYPE_UNDEFINED: Forward declaration without definition */
struct undefined_type;

/* TYPE_SCALAR: Basic typedefs */
typedef int my_scalar;
typedef unsigned int my_unsigned_scalar;
typedef char my_char_scalar;

/* TYPE_STRING: String type with GTY((string)) */
typedef const char *my_string GTY((string));

/* TYPE_STRUCT: Regular structures with GTY(()) */
struct my_struct GTY(()) {
    int field1;
    char field2;
};

struct another_struct GTY(()) {
    long field1;
    double field2;
};

/* TYPE_USER_STRUCT: Structures with GTY((user)) */
struct user_struct GTY((user)) {
    void *data;
    int size;
};

struct custom_user_struct GTY((user)) {
    int id;
    char *name;
};

/* TYPE_UNION: Union definitions */
union my_union GTY(()) {
    int as_int;
    float as_float;
    char as_char[4];
};

union another_union {
    long as_long;
    double as_double;
};

/* TYPE_POINTER: Pointer typedefs */
typedef struct my_struct *my_struct_ptr;
typedef int *int_ptr;
typedef void (*void_func_ptr)(void);

/* TYPE_ARRAY: Array type definitions */
typedef int int_array[10];
typedef char char_array[256];
typedef struct my_struct struct_array[5];

/* TYPE_CALLBACK: Function pointer typedef */
typedef int (*callback_func)(int, void*);
typedef void (*simple_callback)(void);

/* TYPE_LANG_STRUCT: Language-specific structure */
#ifdef GCC
struct lang_specific_struct GTY(()) {
    int lang_field1;
    void *lang_field2;
};
#endif

/* Nested structure for additional coverage */
struct outer_struct GTY(()) {
    struct inner_struct GTY(()) {
        int nested_field;
        char *nested_string GTY((string));
    } inner;
    
    union nested_union {
        int option1;
        float option2;
    } choice;
    
    callback_func handler;
    int_array numbers;
};

/* Enumeration (should be treated as scalar) */
typedef enum {
    VALUE_A,
    VALUE_B,
    VALUE_C
} my_enum;

/* Complex pointer chain */
typedef struct my_struct ***complex_ptr;

/* Array of pointers */
typedef struct my_struct *ptr_array[20];

/* Callback with complex signature */
typedef struct my_struct *(*factory_callback)(int, const char*);

#endif /* TEST_STRUCTURES_H */

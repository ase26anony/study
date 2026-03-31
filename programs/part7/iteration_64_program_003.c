#ifndef TEST_GTY_TYPES_H
#define TEST_GTY_TYPES_H

/* TYPE_UNDEFINED: Forward declaration of incomplete type */
struct opaque;
typedef struct opaque *opaque_ptr_t;

/* TYPE_SCALAR: Basic scalar types */
typedef enum color { RED, GREEN, BLUE } color_t;
typedef int scalar_int;
typedef double scalar_double;

/* TYPE_STRING: String types */
typedef const char *const_string_t;
typedef char *mutable_string_t;

/* TYPE_CALLBACK: Function pointer types */
typedef void (*simple_callback)(void);
typedef int (*complex_callback)(const char *, int);

/* TYPE_POINTER: Various pointer types */
typedef scalar_int *int_ptr_t;
typedef void *generic_ptr_t;

/* TYPE_ARRAY: Array types */
typedef int fixed_array_t[10];
typedef char *string_array_t[5];

/* TYPE_STRUCT: Basic structure */
struct basic_struct {
    int id;
    char *name;
    double value;
};

/* TYPE_UNION: Basic union */
union basic_union {
    int as_int;
    double as_double;
    char *as_string;
};

/* TYPE_USER_STRUCT: User-defined structure with custom traversal */
struct user_defined {
    int magic;
    void *user_data;
};

/* GTY markers for user structures */
#define GTY(x) __attribute__((gty(x)))

#endif /* TEST_GTY_TYPES_H */

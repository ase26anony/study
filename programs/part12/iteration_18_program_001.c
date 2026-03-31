#ifndef TEST_TYPES_H
#define TEST_TYPES_H

/* Scalar types */
typedef int my_int_t;
typedef float my_float_t;
typedef double my_double_t;
typedef char my_char_t;
typedef long long my_llong_t;
typedef _Bool my_bool_t;

/* String type */
typedef char* string_t;

/* Struct types */
struct SimpleStruct {
    int x;
    float y;
    char z;
};

/* User struct type (typedef'd struct) */
typedef struct {
    double a;
    int b;
    char c[20];
} UserStruct;

/* Union types */
union SimpleUnion {
    int as_int;
    float as_float;
    void* as_ptr;
};

/* Typedef'd union */
typedef union {
    long long ll;
    double d;
    char* str;
} TypedefUnion;

/* Pointer types */
typedef int* int_ptr_t;
typedef void** void_double_ptr_t;

/* Array types */
typedef int int_array_t[10];
typedef struct SimpleStruct struct_array_t[5];

/* Callback types */
typedef void (*simple_callback_t)(int);
typedef int (*complex_callback_t)(void*, const char*, int);

/* Language-specific struct (GCC extension) */
struct LangStruct {
    int data;
    void* ptr;
} __attribute__((packed));

/* Transparent union (another GCC extension) */
typedef union __attribute__((transparent_union)) {
    int* int_ptr;
    void* void_ptr;
} TransparentUnion;

/* Function pointer returning struct */
typedef struct SimpleStruct (*struct_returning_func_t)(int);

/* Multi-dimensional array */
typedef int matrix_t[3][3];

/* Forward declarations to ensure TYPE_UNDEFINED appears */
struct ForwardDeclared;
union ForwardUnion;

/* Enum type (scalar category) */
typedef enum {
    RED,
    GREEN,
    BLUE
} Color;

#endif /* TEST_TYPES_H */

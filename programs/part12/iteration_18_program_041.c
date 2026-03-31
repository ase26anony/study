/* types.h - Header file with type declarations */
#ifndef TYPES_H
#define TYPES_H

/* Scalar types */
typedef int my_int_t;
typedef float my_float_t;
typedef double my_double_t;
typedef char my_char_t;
typedef long long my_llong_t;
typedef _Bool my_bool_t;

/* String type */
typedef char* my_string_t;

/* Struct types */
struct SimpleStruct {
    int x;
    float y;
    char z;
};

/* User struct type (typedef'd struct) */
typedef struct {
    int id;
    char name[32];
    float score;
} UserStruct;

/* Union types */
union SimpleUnion {
    int as_int;
    float as_float;
    char as_char;
};

/* Typedef'd union */
typedef union {
    long long timestamp;
    double value;
    void* ptr;
} TypedefUnion;

/* Pointer types */
typedef int* int_ptr_t;
typedef int** int_dbl_ptr_t;
typedef struct SimpleStruct* struct_ptr_t;

/* Array types */
typedef int int_array_t[10];
typedef struct SimpleStruct struct_array_t[5];

/* Callback types */
typedef void (*simple_callback_t)(int);
typedef int (*complex_callback_t)(int, float, void*);

/* Language-specific struct (GCC extension) */
struct LangStruct {
    int a;
    char b;
    long c;
} __attribute__((packed));

/* Transparent union (another GCC extension) */
typedef union {
    int* as_int_ptr;
    float* as_float_ptr;
} TransparentUnion __attribute__((transparent_union));

#endif /* TYPES_H */

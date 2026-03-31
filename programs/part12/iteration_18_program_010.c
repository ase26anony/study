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
typedef char* string_t;

/* Struct types */
struct SimpleStruct {
    int x;
    float y;
    char z;
};

/* User struct type (typedef'd struct) */
typedef struct {
    double data;
    int tag;
} UserStruct;

/* Another struct with complex members */
struct ComplexStruct {
    struct SimpleStruct simple;
    UserStruct user;
    long long extra;
};

/* Union types */
union SimpleUnion {
    int as_int;
    float as_float;
    void* as_ptr;
};

/* Typedef'd union */
typedef union {
    char bytes[8];
    long long value;
} ByteUnion;

/* Pointer types */
typedef int* int_ptr_t;
typedef int** int_dbl_ptr_t;
typedef struct SimpleStruct* struct_ptr_t;

/* Array types */
typedef int int_array_10[10];
typedef struct SimpleStruct struct_array_5[5];
typedef int* pointer_array_20[20];

/* Callback types */
typedef void (*simple_callback)(int);
typedef int (*complex_callback)(struct SimpleStruct*, UserStruct);

/* Language-specific struct (GCC extension) */
struct LangStruct {
    int data;
    char more;
} __attribute__((packed));

/* Transparent union (another GCC extension) */
typedef union __attribute__((transparent_union)) {
    int* int_ptr;
    void* void_ptr;
} TransparentUnion;

#endif /* TYPES_H */

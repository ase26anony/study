/* test_types.h - Comprehensive type definitions for gengtype coverage testing */

#ifndef TEST_TYPES_H
#define TEST_TYPES_H

/* TYPE_UNDEFINED: Forward declaration of incomplete type */
struct incomplete;  /* This should trigger TYPE_UNDEFINED */

/* TYPE_SCALAR: Fundamental scalar types */
typedef int int_t;
typedef float float_t;
typedef double double_t;
typedef char char_t;
typedef long long long_long_t;
typedef unsigned int uint_t;
typedef _Bool bool_t;

/* TYPE_STRING: String types */
typedef char* string_t;
typedef const char* const_string_t;
typedef char* mutable_string_t;

/* TYPE_STRUCT: Plain C structures */
struct Point {
    int x;
    int y;
    float z;
};

struct Rectangle {
    struct Point top_left;
    struct Point bottom_right;
    double area;
};

/* Nested structure */
struct Employee {
    char name[50];
    int id;
    struct Point location;
    double salary;
};

/* TYPE_UNION: Union types */
union Data {
    int i;
    float f;
    char str[20];
    double d;
};

union Variant {
    long long int_val;
    double float_val;
    void* ptr_val;
    char str_val[32];
};

/* TYPE_POINTER: Pointer types to various entities */
typedef struct Point* PointPtr;
typedef union Data* DataPtr;
typedef int_t* IntPtr;
typedef string_t* StringPtrPtr;
typedef void (*VoidFuncPtr)(void);

/* Pointer to pointer */
typedef PointPtr* PointPtrPtr;

/* TYPE_ARRAY: Array types */
typedef int IntArray[10];
typedef struct Point PointArray[5];
typedef char CharMatrix[3][4];
typedef double Double3D[2][3][4];

/* Fixed-size arrays */
int global_array[100];
static const float const_array[] = {1.0f, 2.0f, 3.0f};

/* TYPE_CALLBACK: Function pointer types */
typedef int (*Comparator)(const void*, const void*);
typedef void (*Callback)(int, void*);
typedef char* (*StringProcessor)(const char*);
typedef int (*BinaryOp)(int, int);

/* More complex callback */
typedef void (*ErrorHandler)(const char* file, int line, const char* msg);

/* TYPE_LANG_STRUCT: GCC-specific attributed structures */
struct __attribute__((aligned(16))) AlignedStruct {
    int data[4];
    double precision;
};

struct __attribute__((packed)) PackedStruct {
    char a;
    int b;
    char c;
};

struct __attribute__((transparent_union)) TransparentUnion {
    int i;
    float f;
};

/* Vector types (GCC extension) */
typedef int v4si __attribute__((vector_size(16)));

/* TYPE_USER_STRUCT: May be triggered by typedef'd structs */
typedef struct Point PointType;
typedef struct Rectangle RectType;

/* Complex type combinations */
struct ComplexType {
    /* Contains all type categories */
    int scalar;                    /* TYPE_SCALAR */
    char* string;                  /* TYPE_STRING */
    struct Point point;            /* TYPE_STRUCT */
    union Data data;               /* TYPE_UNION */
    int* pointer;                  /* TYPE_POINTER */
    int array[10];                 /* TYPE_ARRAY */
    Comparator compare;            /* TYPE_CALLBACK */
    v4si vector;                   /* TYPE_LANG_STRUCT (vector type) */
};

/* Forward declarations that remain incomplete */
struct forward_declared;
typedef struct forward_declared ForwardType;

/* Opaque pointer type */
typedef struct opaque* OpaqueHandle;

#endif /* TEST_TYPES_H */

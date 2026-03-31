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
typedef long long longlong_t;
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
    double z;
};

struct Rectangle {
    struct Point top_left;
    struct Point bottom_right;
    int area;
};

/* Nested structure */
struct Employee {
    char name[50];
    int id;
    struct Point location;
    float salary;
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
    char char_val;
};

/* TYPE_POINTER: Pointer types to various entities */
typedef struct Point* PointPtr;
typedef union Data* DataPtr;
typedef int_t* IntPtr;
typedef void (*VoidFuncPtr)(void);
typedef const char** StringArrayPtr;

/* TYPE_ARRAY: Array types */
typedef int IntArray[10];
typedef struct Point PointArray[5];
typedef char CharMatrix[3][3];
typedef union Data DataArray[8];

/* Fixed-size arrays */
int global_array[100];
static const float const_array[] = {1.0f, 2.0f, 3.0f};

/* TYPE_CALLBACK: Function pointer types */
typedef int (*Comparator)(const void*, const void*);
typedef void (*Callback)(int, void*);
typedef char* (*StringProcessor)(const char*);
typedef int (*BinaryOp)(int, int);

/* TYPE_LANG_STRUCT: Struct with GCC attributes (may trigger special handling) */
struct __attribute__((aligned(16))) AlignedStruct {
    int data[4];
    char padding[12];
};

struct __attribute__((packed)) PackedStruct {
    char a;
    int b;
    short c;
};

/* Complex type with multiple attributes */
struct __attribute__((aligned(32), transparent_union)) SpecialStruct {
    union {
        int i;
        float f;
    } value;
};

/* Forward declaration that might remain incomplete */
struct forward_declared;

/* Opaque pointer type */
typedef struct forward_declared* OpaqueHandle;

#endif /* TEST_TYPES_H */

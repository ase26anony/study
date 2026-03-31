/* test_types.h - Comprehensive type definitions to cover all gengtype classifications */

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
    double z;
};

struct Rectangle {
    struct Point top_left;
    struct Point bottom_right;
    int id;
};

/* Nested structure */
struct Container {
    struct Point point;
    struct Rectangle rect;
    int tag;
};

/* TYPE_UNION: Union types */
union Data {
    int i;
    float f;
    double d;
    char str[32];
};

union Variant {
    struct Point as_point;
    struct Rectangle as_rect;
    union Data as_data;
    long long as_ll;
};

/* TYPE_POINTER: Pointer types to various entities */
typedef struct Point* PointPtr;
typedef struct Rectangle* RectPtr;
typedef union Data* DataPtr;
typedef int_t* IntPtr;
typedef float_t* FloatPtr;
typedef void* GenericPtr;
typedef const struct Point* ConstPointPtr;

/* TYPE_ARRAY: Array types */
typedef int IntArray[10];
typedef struct Point PointArray[5];
typedef union Data DataArray[8];
typedef char CharMatrix[3][4];
typedef int* PointerArray[6];

/* TYPE_CALLBACK: Function pointer types */
typedef int (*Comparator)(const void*, const void*);
typedef void (*Callback)(int, void*);
typedef struct Point* (*PointFactory)(int, int);
typedef int (*BinaryOp)(int, int);

/* TYPE_LANG_STRUCT: Struct with GCC attributes (potential TYPE_LANG_STRUCT) */
struct __attribute__((aligned(16))) AlignedStruct {
    int data;
    double precise;
    char padding[8];
};

struct __attribute__((packed)) PackedStruct {
    char a;
    int b;
    char c;
};

/* Complex type with multiple attributes */
struct __attribute__((aligned(32), packed)) ComplexAlignedStruct {
    long long data[4];
    char metadata[16];
};

/* More complex type combinations */
typedef struct {
    int id;
    char name[32];
    union {
        int int_value;
        double double_value;
    } value;
    struct {
        unsigned int flags;
        void* next;
    } meta;
} ComplexType;

/* Function pointer returning pointer to struct */
typedef struct Point* (*GetPointFunc)(void);

/* Array of function pointers */
typedef int (*MathFunc[4])(int, int);

#endif /* TEST_TYPES_H */

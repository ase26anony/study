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

typedef struct {
    int id;
    string_t name;
    float score;
} Student;

/* TYPE_UNION: Union types */
union Data {
    int i;
    float f;
    double d;
    char str[32];
};

union Variant {
    long long as_int;
    double as_float;
    void* as_pointer;
    struct Point as_point;
};

/* TYPE_POINTER: Pointer types to various entities */
typedef struct Point* PointPtr;
typedef union Data* DataPtr;
typedef int_t* IntPtr;
typedef void (*VoidFuncPtr)(void);
typedef const_string_t* StringPtrPtr;

/* TYPE_ARRAY: Array types */
typedef int IntArray[10];
typedef struct Point PointArray[5];
typedef union Data DataArray[3];
typedef char CharMatrix[4][4];
typedef int (*FuncPtrArray[5])(int, int);

/* TYPE_CALLBACK: Function pointer types */
typedef int (*Comparator)(const void*, const void*);
typedef void (*Callback)(int, void*);
typedef string_t (*StringGenerator)(int);
typedef int (*BinaryOp)(int, int);

/* TYPE_LANG_STRUCT: Struct with GCC attributes (potential TYPE_LANG_STRUCT) */
struct __attribute__((aligned(16))) AlignedStruct {
    int data[4];
    double precision;
};

struct __attribute__((packed)) PackedStruct {
    char a;
    int b;
    char c;
};

/* Complex nested type to ensure thorough analysis */
typedef struct ComplexType {
    union Data data_union;
    struct Point points[3];
    Comparator compare_func;
    void* user_data;
    int (*operations[3])(struct ComplexType*);
} ComplexType;

/* Forward pointer typedef (another potential TYPE_UNDEFINED trigger) */
typedef struct ForwardDeclared* ForwardPtr;

/* Additional struct that uses forward declaration */
struct Container {
    struct incomplete* unknown;  /* TYPE_UNDEFINED usage */
    ForwardPtr forward;          /* TYPE_POINTER to undefined */
    ComplexType complex;         /* TYPE_STRUCT */
};

#endif /* TEST_TYPES_H */

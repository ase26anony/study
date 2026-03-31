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
    struct {
        int day;
        int month;
        int year;
    } hire_date;
    float salary;
};

/* TYPE_UNION: Union types */
union Data {
    int i;
    float f;
    double d;
    char str[20];
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
typedef int* IntPtr;
typedef char** StringArrayPtr;
typedef void (*VoidFuncPtr)(void);

/* Pointer to pointer */
typedef PointPtr* PointPtrPtr;

/* TYPE_ARRAY: Array types */
typedef int IntArray[10];
typedef struct Point PointArray[5];
typedef char CharMatrix[3][3];
typedef double Double3D[2][2][2];

/* Fixed-size string array */
typedef char StringTable[5][50];

/* TYPE_CALLBACK: Function pointer types */
typedef int (*Comparator)(const void*, const void*);
typedef void (*Callback)(int, void*);
typedef char* (*StringProcessor)(const char*);
typedef int (*BinaryOp)(int, int);

/* Complex callback with struct parameter */
typedef struct Point (*PointGenerator)(int, int);

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

/* Struct with vector attribute */
struct __attribute__((vector_size(16))) VectorStruct {
    float elements[4];
};

/* TYPE_USER_STRUCT: May be triggered by typedef'd structs */
typedef struct Point PointType;
typedef struct Rectangle RectType;
typedef struct Employee EmpType;

/* Complete the incomplete type */
struct incomplete {
    int data;
    void* next;
};

/* Additional complex types for thorough testing */

/* Function type */
typedef int FuncType(int, int);

/* Anonymous struct in union */
union AnonymousUnion {
    struct {
        int x;
        int y;
    } point;
    struct {
        float r;
        float g;
        float b;
    } color;
};

/* Const qualified types */
typedef const int ConstInt;
typedef const struct Point ConstPoint;
typedef int* const ConstPtrToInt;

/* Volatile qualified types */
typedef volatile int VolatileInt;

/* Restrict qualified pointer */
typedef int* __restrict__ RestrictIntPtr;

#endif /* TEST_TYPES_H */

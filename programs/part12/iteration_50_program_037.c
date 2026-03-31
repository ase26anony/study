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
    int id;
};

/* Nested structure */
struct Employee {
    char name[50];
    int age;
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
    char char_val;
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
typedef union Data DataArray[8];

/* Fixed-size arrays */
int global_array[20];
static const float const_array[5] = {1.0, 2.0, 3.0, 4.0, 5.0};

/* TYPE_CALLBACK: Function pointer types */
typedef int (*Comparator)(const void*, const void*);
typedef void (*Callback)(int, char*);
typedef int (*BinaryOp)(int, int);
typedef char* (*StringProcessor)(const char*);
typedef void (*ErrorHandler)(int, const char*);

/* Complex callback type */
typedef int (*SortFunction)(void**, size_t, Comparator);

/* TYPE_LANG_STRUCT: GCC-specific attributed structures */
struct __attribute__((aligned(16))) AlignedStruct {
    int data[4];
    char padding[12];
};

struct __attribute__((packed)) PackedStruct {
    char a;
    int b;
    short c;
};

struct __attribute__((transparent_union)) TransparentUnion {
    int i;
    float f;
};

/* Vector types (GCC extension) */
typedef int v4si __attribute__((vector_size(16)));

/* TYPE_USER_STRUCT: May be triggered by typedef'd structs */
typedef struct Point PointType;
typedef struct Employee EmployeeType;

/* Additional incomplete types for TYPE_UNDEFINED */
union forward_union;
enum forward_enum;

#endif /* TEST_TYPES_H */

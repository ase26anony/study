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
    float z;
};

struct Rectangle {
    struct Point top_left;
    struct Point bottom_right;
    double area;
};

typedef struct {
    int id;
    char name[50];
    float salary;
} Employee;

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
typedef void (*FuncPtr)(void);

/* TYPE_ARRAY: Array types */
typedef int IntArray[10];
typedef struct Point PointArray[5];
typedef char CharMatrix[3][3];
typedef union Data DataArray[8];

/* TYPE_CALLBACK: Function pointer types */
typedef int (*Comparator)(const void*, const void*);
typedef void (*Callback)(int, char*);
typedef double (*MathFunc)(double);
typedef int (*BinaryOp)(int, int);

/* TYPE_LANG_STRUCT: Struct with GCC attributes (potential TYPE_LANG_STRUCT) */
struct __attribute__((aligned(16))) AlignedStruct {
    int data[4];
    char metadata[32];
};

struct __attribute__((packed)) PackedStruct {
    char a;
    int b;
    short c;
};

/* Complex nested type to ensure thorough analysis */
typedef struct ComplexType {
    int id;
    union Data data;
    struct Point location;
    IntArray scores;
    Comparator compare;
    struct ComplexType* next;
} ComplexType;

/* Function declarations using various types */
void process_point(struct Point* p);
int compare_points(const void* a, const void* b);
union Data create_data(int type, void* value);
ComplexType* create_complex_type(int id);

#endif /* TEST_TYPES_H */

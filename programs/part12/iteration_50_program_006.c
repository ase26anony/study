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
typedef void (*FuncPtr)(void);
typedef const char** StringArrayPtr;

/* TYPE_ARRAY: Array types */
typedef int IntArray[10];
typedef struct Point PointArray[5];
typedef char CharMatrix[3][3];
typedef union Data DataArray[8];

/* TYPE_CALLBACK: Function pointer types */
typedef int (*Comparator)(const void*, const void*);
typedef void (*Callback)(int, const char*);
typedef double (*MathFunc)(double);
typedef int (*BinaryOp)(int, int);

/* TYPE_LANG_STRUCT: Structure with GCC attributes */
struct __attribute__((aligned(16))) AlignedStruct {
    int data[4];
    char padding[12];
};

struct __attribute__((packed)) PackedStruct {
    char a;
    int b;
    short c;
};

/* TYPE_USER_STRUCT: Typedef'd struct */
typedef struct {
    int id;
    char name[32];
} UserDefined;

/* Additional complex types for thorough testing */
struct ComplexType {
    /* Mix of different type categories */
    int scalar_field;
    char* string_field;
    struct Point struct_field;
    union Data union_field;
    int (*callback_field)(int, int);
    int array_field[10];
    struct ComplexType* self_ptr;
};

/* Function prototypes using various types */
void process_point(struct Point* p);
int compare_data(const union Data* d1, const union Data* d2);
double calculate_area(struct Rectangle r);
char* format_string(const char* fmt, ...);

#endif /* TEST_TYPES_H */

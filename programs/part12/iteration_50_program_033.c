/* test_types.h - Comprehensive type definitions to cover all gengtype classifications */

#ifndef TEST_TYPES_H
#define TEST_TYPES_H

/* TYPE_UNDEFINED: Forward declaration of incomplete type */
struct incomplete;

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

typedef struct {
    char name[50];
    int age;
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
    long l;
    void* ptr;
    struct Point point;
};

/* TYPE_POINTER: Pointer types to various entities */
typedef struct Point* PointPtr;
typedef union Data* DataPtr;
typedef int_t* IntPtr;
typedef void (*VoidFunc)(void);
typedef const_string_t* StringPtrPtr;

/* TYPE_ARRAY: Array types */
typedef int IntArray[10];
typedef struct Point PointArray[5];
typedef char CharMatrix[3][4];
typedef union Data DataArray[8];

/* TYPE_CALLBACK: Function pointer types */
typedef int (*Comparator)(const void*, const void*);
typedef void (*Callback)(int, char*);
typedef int (*BinaryOp)(int, int);
typedef void (*ErrorHandler)(const char*, int);

/* TYPE_LANG_STRUCT: GCC-specific attributed structures */
struct __attribute__((aligned(16))) AlignedStruct {
    int data[4];
    char metadata[32];
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

/* Complex nested type to ensure thorough analysis */
typedef struct ComplexType {
    union Data data;
    struct Point position;
    IntArray scores;
    Comparator compare;
    struct ComplexType* next;
    char name[50];
} ComplexType;

/* Function prototypes using various types */
void process_point(struct Point* p);
int compare_points(const void* a, const void* b);
union Data create_data(int type, void* value);
ComplexType* create_complex(const char* name, int* scores, int count);

#endif /* TEST_TYPES_H */

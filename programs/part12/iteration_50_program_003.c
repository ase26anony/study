/* test_types.h - Comprehensive type definitions for gengtype coverage testing */

#ifndef TEST_TYPES_H
#define TEST_TYPES_H

/* TYPE_UNDEFINED: Forward declaration of incomplete type */
struct incomplete;  /* This should trigger TYPE_UNDEFINED */

/* TYPE_SCALAR: Fundamental scalar types */
typedef int my_int_t;
typedef float my_float_t;
typedef double my_double_t;
typedef char my_char_t;
typedef long long my_longlong_t;
typedef unsigned int my_uint_t;
typedef _Bool my_bool_t;

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
    char str[20];
    double d;
};

union Variant {
    long long int_value;
    double float_value;
    void* pointer_value;
    char string_value[32];
};

/* TYPE_POINTER: Pointer types to various entities */
typedef struct Point* PointPtr;
typedef union Data* DataPtr;
typedef int* IntPtr;
typedef char** StringPtrPtr;
typedef void (*VoidFuncPtr)(void);
typedef const struct Rectangle* ConstRectPtr;

/* TYPE_ARRAY: Array types */
typedef int IntArray[10];
typedef struct Point PointArray[5];
typedef char CharMatrix[3][4];
typedef double Double3D[2][3][4];

/* Fixed-size arrays */
int global_array[100];
static float static_float_array[20];
extern double extern_double_array[50];

/* TYPE_CALLBACK: Function pointer types */
typedef int (*Comparator)(const void*, const void*);
typedef void (*Callback)(int, char*);
typedef char* (*StringProcessor)(const char*);
typedef int (*BinaryOp)(int, int);

/* Complex callback with structure parameter */
typedef struct Point (*PointGenerator)(int, int);
typedef void (*PointProcessor)(struct Point*);

/* TYPE_LANG_STRUCT: Structures with GCC attributes */
struct __attribute__((aligned(16))) AlignedStruct {
    int data;
    char padding[12];
};

struct __attribute__((packed)) PackedStruct {
    char a;
    int b;
    char c;
};

struct __attribute__((transparent_union)) TransparentUnion {
    union {
        int i;
        float f;
    } u;
};

/* Vector types (may be treated specially) */
typedef int __attribute__((vector_size(16))) v4si;
typedef float __attribute__((vector_size(32))) v8sf;

/* TYPE_USER_STRUCT: This might be triggered by typedef'd structs */
typedef struct Point PointType;
typedef struct Employee EmployeeType;

/* Anonymous struct/union in typedef */
typedef struct {
    int tag;
    union {
        int num;
        char* str;
    } value;
} TaggedValue;

/* Forward declared struct that becomes complete later */
struct LaterDefined;
struct LaterDefined {
    int value;
    struct LaterDefined* next;
};

/* Enumeration types */
enum Color { RED, GREEN, BLUE };
typedef enum Color ColorType;

/* Bitfield structure */
struct BitFields {
    unsigned int flag1 : 1;
    unsigned int flag2 : 3;
    unsigned int flag3 : 4;
    unsigned int padding : 24;
};

#endif /* TEST_TYPES_H */

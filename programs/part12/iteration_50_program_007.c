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
struct Container {
    struct Point point;
    struct Rectangle rect;
    int count;
};

/* TYPE_UNION: Union types */
union Data {
    int i;
    float f;
    double d;
    char str[32];
};

union Variant {
    long long int_val;
    double float_val;
    void* ptr_val;
    struct Point point_val;
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
typedef union Data DataArray[3];
typedef char CharMatrix[4][4];

/* Fixed-size string array */
typedef char FixedString[256];

/* TYPE_CALLBACK: Function pointer types */
typedef int (*Comparator)(const void*, const void*);
typedef void (*Callback)(int, void*);
typedef string_t (*StringGenerator)(void);
typedef int (*BinaryOp)(int, int);

/* Complex callback with struct parameter */
typedef struct Point (*PointFactory)(int, int, double);

/* TYPE_LANG_STRUCT: GCC-specific attributed structures */
struct __attribute__((aligned(16))) AlignedStruct {
    int data;
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

/* TYPE_USER_STRUCT: Typedef'd structs */
typedef struct Point PointType;
typedef struct Rectangle RectType;

/* Additional complex types for thorough testing */

/* Function type */
typedef int FuncType(int, int);

/* Const pointer types */
typedef const struct Point* ConstPointPtr;
typedef const int* ConstIntPtr;

/* Volatile types */
typedef volatile int VolatileInt;
typedef volatile struct Point* VolatilePointPtr;

/* Restrict-qualified pointer */
typedef int* __restrict__ RestrictIntPtr;

/* Anonymous struct/union */
struct AnonymousContainer {
    struct {
        int x;
        int y;
    } coord;
    union {
        int i;
        float f;
    } value;
};

/* Bitfield structure */
struct BitfieldStruct {
    unsigned int flag1 : 1;
    unsigned int flag2 : 3;
    unsigned int flag3 : 4;
    int value : 16;
    unsigned int : 12;  /* Padding */
};

/* Forward declared struct that will be completed later */
struct ForwardDeclared;

/* Opaque pointer type */
typedef struct ForwardDeclared* OpaqueHandle;

/* Complete the forward declaration */
struct ForwardDeclared {
    OpaqueHandle next;
    int data;
    struct Point position;
};

/* Enumeration type */
enum Color {
    RED,
    GREEN,
    BLUE,
    ALPHA = 255
};

typedef enum Color Color_t;

/* Complex nested type */
typedef struct {
    struct {
        PointArray points;
        int count;
    } geometry;
    union {
        Color_t color;
        int color_code;
    } appearance;
    Comparator compare_func;
    void* user_data;
} ComplexType;

#endif /* TEST_TYPES_H */

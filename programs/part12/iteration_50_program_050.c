/* test_types.h - Comprehensive type definitions for gengtype coverage testing */

#ifndef TEST_TYPES_H
#define TEST_TYPES_H

/* TYPE_UNDEFINED: Forward declaration of incomplete type */
struct incomplete;

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
typedef char* const const_string_ptr_t;

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

/* TYPE_USER_STRUCT: Typedef'd structures */
typedef struct Point Point_t;
typedef struct Rectangle Rect_t;

/* TYPE_UNION: Union types */
union Data {
    int i;
    float f;
    double d;
    char str[32];
};

union Variant {
    long long ll;
    void* ptr;
    struct Point point;
};

/* TYPE_POINTER: Pointer types to various entities */
typedef struct Point* PointPtr;
typedef union Data* DataPtr;
typedef int* IntPtr;
typedef char** StringArrayPtr;
typedef void (*VoidFuncPtr)(void);

/* TYPE_ARRAY: Array types */
typedef int IntArray[10];
typedef struct Point PointArray[5];
typedef char CharMatrix[3][4];
typedef union Data DataArray[8];

/* TYPE_CALLBACK: Function pointer types */
typedef int (*Comparator)(const void*, const void*);
typedef void (*EventHandler)(int event_id, void* user_data);
typedef char* (*StringFormatter)(int value, const char* format);
typedef int (*BinaryOp)(int a, int b);

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

/* Complex nested type to ensure thorough analysis */
typedef struct ComplexType {
    union Data data;
    struct Point position;
    IntArray values;
    Comparator compare_func;
    struct ComplexType* next;
    char name[64];
} ComplexType_t;

/* Enumeration types (should be treated as scalar) */
typedef enum Color {
    RED,
    GREEN,
    BLUE,
    ALPHA = 255
} Color_t;

/* Function prototypes using various types */
void process_point(struct Point* p);
int compare_points(const void* a, const void* b);
union Data create_data(int type, void* value);
ComplexType_t* create_complex_type(const char* name);

#endif /* TEST_TYPES_H */

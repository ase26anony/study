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
    int id;
};

/* Nested structure */
struct Container {
    struct Point point;
    struct Rectangle rect;
    char name[32];
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
    struct Point point_val;
};

/* TYPE_POINTER: Pointer types to various entities */
typedef struct Point* PointPtr;
typedef union Data* DataPtr;
typedef int* IntPtr;
typedef char** StringPtrPtr;
typedef void (*FuncPtr)(void);
typedef struct Container* ContainerPtr;

/* TYPE_ARRAY: Array types */
typedef int IntArray[10];
typedef struct Point PointArray[5];
typedef char CharMatrix[3][4];
typedef union Data DataArray[8];

/* Fixed-size arrays */
extern int global_array[100];
extern struct Point point_buffer[50];
extern const char* string_array[20];

/* TYPE_CALLBACK: Function pointer types */
typedef int (*Comparator)(const void*, const void*);
typedef void (*Callback)(int, void*);
typedef char* (*StringProcessor)(const char*);
typedef struct Point (*PointGenerator)(int, int);

/* TYPE_LANG_STRUCT: GCC-specific structures with attributes */
struct __attribute__((aligned(16))) AlignedStruct {
    int data;
    char padding[12];
};

struct __attribute__((packed)) PackedStruct {
    char a;
    int b;
    short c;
};

struct __attribute__((transparent_union)) TransparentUnion {
    union {
        int i;
        float f;
    } u;
};

/* Complex type with multiple attributes */
struct __attribute__((aligned(32), packed)) ComplexAlignedStruct {
    long long data1;
    int data2;
    char data3;
};

/* TYPE_USER_STRUCT: Typedef'd struct (may be treated as user struct) */
typedef struct {
    int id;
    char name[64];
    float score;
} UserDefined;

/* Additional pointer types for coverage */
typedef UserDefined* UserPtr;
typedef union Variant* VariantPtr;

/* Function declarations using the types */
void process_point(struct Point* p);
int compare_points(const void* a, const void* b);
union Data create_data(int type, void* value);
void iterate_array(IntArray arr, int size);

#endif /* TEST_TYPES_H */

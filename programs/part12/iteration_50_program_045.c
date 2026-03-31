/* test_types.h - Comprehensive type definitions for gengtype coverage testing */

#ifndef TEST_TYPES_H
#define TEST_TYPES_H

/* TYPE_UNDEFINED: Forward declaration of incomplete type */
struct incomplete;
typedef struct incomplete incomplete_t;

/* TYPE_SCALAR: Fundamental scalar types */
typedef int scalar_int_t;
typedef float scalar_float_t;
typedef double scalar_double_t;
typedef char scalar_char_t;
typedef long long scalar_longlong_t;
typedef unsigned int scalar_uint_t;
typedef _Bool scalar_bool_t;

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

typedef struct Point Point_t;
typedef struct Rectangle Rectangle_t;

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
};

typedef union Data Data_t;
typedef union Variant Variant_t;

/* TYPE_POINTER: Pointer types to various entities */
typedef int* int_ptr_t;
typedef struct Point* PointPtr;
typedef union Data* DataPtr;
typedef void* generic_ptr_t;
typedef const struct Rectangle* ConstRectPtr;
typedef scalar_double_t* double_ptr_t;

/* TYPE_ARRAY: Array types */
typedef int int_array_10[10];
typedef struct Point point_array_5[5];
typedef char char_array_256[256];
typedef const_string_t string_array_8[8];
typedef int multi_dim_array[3][4][5];

/* TYPE_CALLBACK: Function pointer types */
typedef int (*comparator_t)(const void*, const void*);
typedef void (*callback_t)(int, void*);
typedef char* (*string_generator_t)(void);
typedef int (*binary_op_t)(int, int);
typedef void (*error_handler_t)(const char*, int);

/* TYPE_LANG_STRUCT: Struct with GCC attributes (potential lang_struct) */
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
    int i;
    float f;
};

/* Complex nested type combinations */
struct ComplexType {
    int id;
    char name[64];
    struct Point location;
    union Data value;
    int* references;
    int (*compare)(struct ComplexType*, struct ComplexType*);
    struct ComplexType* next;
};

/* Function types for TYPE_CALLBACK coverage */
typedef struct ComplexType* (*complex_allocator_t)(void);
typedef void (*complex_deallocator_t)(struct ComplexType*);

/* Enum types (may be treated as scalar) */
typedef enum Color {
    RED,
    GREEN,
    BLUE,
    ALPHA
} Color_t;

typedef enum __attribute__((packed)) SmallEnum {
    VALUE_A = 1,
    VALUE_B = 2,
    VALUE_C = 3
} SmallEnum_t;

/* Bitfield struct */
struct BitfieldStruct {
    unsigned int flag1 : 1;
    unsigned int flag2 : 2;
    unsigned int flag3 : 3;
    unsigned int reserved : 26;
};

/* Volatile and const qualified types */
typedef volatile int volatile_int_t;
typedef const volatile int const_volatile_int_t;

/* Atomic types (C11) */
#ifdef __STDC_NO_ATOMICS__
#else
#include <stdatomic.h>
typedef atomic_int atomic_int_t;
#endif

#endif /* TEST_TYPES_H */

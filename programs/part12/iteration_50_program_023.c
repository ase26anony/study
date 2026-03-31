/* test_types.h - Comprehensive type definitions for gengtype coverage testing */

#ifndef TEST_TYPES_H
#define TEST_TYPES_H

/* TYPE_UNDEFINED: Forward declaration of incomplete type */
struct incomplete;
typedef struct incomplete *incomplete_ptr_t;

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
    int area;
};

typedef struct {
    char name[50];
    int id;
    float score;
} Student;

/* TYPE_UNION: Union types */
union Data {
    int i;
    float f;
    double d;
    char str[20];
};

union Variant {
    long long as_int;
    double as_float;
    void* as_pointer;
    struct Point as_point;
};

/* TYPE_POINTER: Pointer types to various entities */
typedef struct Point* PointPtr;
typedef union Data* DataPtr;
typedef int_t* int_ptr_t;
typedef void (*func_ptr_t)(void);
typedef const_string_t* string_ptr_ptr_t;

/* TYPE_ARRAY: Array types */
typedef int int_array_10[10];
typedef struct Point point_array_5[5];
typedef char char_matrix[3][4];
typedef int (*func_ptr_array[5])(void);

/* TYPE_CALLBACK: Function pointer types */
typedef int (*comparator_t)(const void*, const void*);
typedef void (*callback_t)(int, const char*);
typedef int (*binary_op_t)(int, int);
typedef void (*error_handler_t)(const char*, int);

/* TYPE_LANG_STRUCT: Structs with GCC attributes (potential lang_struct) */
struct __attribute__((aligned(16))) AlignedStruct {
    int data;
    char padding[12];
};

struct __attribute__((packed)) PackedStruct {
    char a;
    int b;
    char c;
};

/* Complex nested type to ensure thorough analysis */
typedef struct ComplexType {
    union Data data;
    struct Point position;
    int_array_10 counts;
    comparator_t compare;
    string_t name;
    struct ComplexType* next;
} ComplexType;

/* Template-like macro for generic containers */
#define DECLARE_CONTAINER(TYPE, NAME) \
    struct NAME { \
        TYPE* items; \
        int size; \
        int capacity; \
    }

DECLARE_CONTAINER(int, IntVector);
DECLARE_CONTAINER(struct Point, PointVector);
DECLARE_CONTAINER(string_t, StringVector);

/* Enumeration type (should be TYPE_SCALAR) */
typedef enum Color {
    RED,
    GREEN,
    BLUE,
    ALPHA = 255
} Color_t;

/* Bitfield structure */
struct BitfieldStruct {
    unsigned int flag1 : 1;
    unsigned int flag2 : 2;
    unsigned int flag3 : 3;
    unsigned int : 2; /* unnamed bitfield */
    unsigned int flag4 : 4;
};

#endif /* TEST_TYPES_H */

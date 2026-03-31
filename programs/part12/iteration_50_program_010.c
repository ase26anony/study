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
    my_int_t count;
};

/* TYPE_UNION: Union types */
union Data {
    int i;
    float f;
    double d;
    char str[32];
};

union Variant {
    struct Point as_point;
    struct Rectangle as_rect;
    union Data as_data;
};

/* TYPE_POINTER: Pointer types to various entities */
typedef struct Point* PointPtr;
typedef struct Rectangle* RectPtr;
typedef union Data* DataPtr;
typedef int* IntPtr;
typedef char** StringArrayPtr;
typedef void* GenericPtr;

/* Pointer to pointer */
typedef PointPtr* PointPtrPtr;

/* TYPE_ARRAY: Array types */
typedef int IntArray[10];
typedef struct Point PointArray[5];
typedef union Data DataArray[8];
typedef char CharMatrix[3][4];

/* Fixed-size string array */
typedef char FixedString[256];

/* TYPE_CALLBACK: Function pointer types */
typedef int (*comparator_t)(const void*, const void*);
typedef void (*callback_t)(int, void*);
typedef char* (*string_generator_t)(void);
typedef int (*binary_op_t)(int, int);

/* More complex callback */
typedef void (*error_handler_t)(const char*, int, void*);

/* TYPE_LANG_STRUCT: Struct with GCC attributes that might trigger special handling */
struct __attribute__((aligned(16))) AlignedStruct {
    int data[4];
    char padding[12];
};

struct __attribute__((packed)) PackedStruct {
    char a;
    int b;
    short c;
};

/* Struct with vector attribute (GCC extension) */
struct __attribute__((vector_size(16))) VectorStruct {
    float elements[4];
};

/* TYPE_USER_STRUCT: Might be triggered by typedef'd structs */
typedef struct Point PointType;
typedef struct Rectangle RectangleType;

/* Complex type combining multiple categories */
struct ComplexType {
    PointPtr points;          /* TYPE_POINTER */
    IntArray values;          /* TYPE_ARRAY */
    comparator_t compare;     /* TYPE_CALLBACK */
    union Data current;       /* TYPE_UNION */
    string_t name;            /* TYPE_STRING */
    int count;                /* TYPE_SCALAR */
    struct AlignedStruct aligned; /* TYPE_LANG_STRUCT */
};

/* Enumeration type (might be classified as scalar) */
typedef enum {
    RED,
    GREEN,
    BLUE
} Color;

/* Function prototypes that use all these types */
void process_point(struct Point* p);
int compare_points(const void* a, const void* b);
union Data create_data(int type, void* value);
void handle_error(const char* msg, int code, void* context);

#endif /* TEST_TYPES_H */
